#pragma once

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <iostream>
#include <string>

#include "ConnectInfo.hpp"
#include "tools.hpp"
#include "UserManager.hpp"
#include "MessagePool.hpp"

#define MAX_ROUND_COUNT 10
#define THREAD_COUNT 1

class TcpConnect
{
    public:
        TcpConnect()
        {
            new_sock_ = -1;
            server_ = NULL;
        }

        ~TcpConnect()
        {}

        void SetSockfd(int fd)
        {
            new_sock_ = fd;
        }

        void SetServer(void* server)
        {
            server_ = server;
        }

        int GetSockfd()
        {
            return new_sock_;
        }

        void* GetServer()
        {
            return server_;
        }
    private:
        int new_sock_;
        //保存ChatServer类的this指针，确保在tcp的线程入口函数当中可以获取到用户管理模块的实例化指针
        void* server_;
};

class ChatServer
{
    public:
        ChatServer()
        {
            tcp_sock_ = -1;
            tcp_port_ = TCP_PORT;
            user_manager_ = NULL;
            udp_sock_ = -1;
            udp_port_ = UDP_PORT;
            memset(con_tid_, '\0', THREAD_COUNT*sizeof(pthread_t));
            memset(pro_tid_, '\0', THREAD_COUNT*sizeof(pthread_t));
            msg_pool_ = NULL;
            udp_msg_ = NULL;
        }

        ~ChatServer()
        {
            if(user_manager_)
            {
                delete user_manager_;
                user_manager_ = NULL;
            }

            if(msg_pool_)
            {
                delete msg_pool_;
                msg_pool_ = NULL;
            }

            if(udp_msg_)
            {
                delete udp_msg_;
                udp_msg_ = NULL;
            }
        }

        /*
         * 初始化变量(服务)的接口，被调用者调用的接口
         * 用户管理模块的实例化对象，消息池的实例化对象
         * */
        int initServer(uint16_t tcp_port = TCP_PORT)
        {
            //创建消息池
            msg_pool_ = new MsgPool(1024);
            if(msg_pool_ == NULL)
            {
                LOG(ERROR, "init msgpool failed") << std::endl;
                return -1;
            }

            // 1.创建tcp-socket，并且绑定地址信息，监听
            // 注册+登录模块
            tcp_sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(tcp_sock_ < 0)
            {
                return -1;
            }

            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(tcp_port);
            /*
             * inet_addr：
             *      1.将点分十进制IP地址转换为uint32_t
             *      2.将uint32_t从主机字节序转换为网络字节序
             *
             *      "0.0.0.0":代表侦听当前机器当中任意网卡信息
             * */
            addr.sin_addr.s_addr = inet_addr("0.0.0.0");

            int ret = bind(tcp_sock_, (struct sockaddr*)&addr, sizeof(addr));
            if(ret < 0)
            {
                return -2;
            }

            ret = listen(tcp_sock_, 5);
            if(ret < 0)
            {
                return -3;
            }

            LOG(INFO, "listen port is 17878") << std::endl;

            user_manager_ = new UserManager();
            if(!user_manager_)
            {
                LOG(INFO, "UserManager") << std::endl;
                return -1;
            }

            udp_sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if(udp_sock_ < 0)
            {
                LOG(ERROR, "create udp socket failed") << std::endl;
                return -1;
            }

            ret = bind(udp_sock_, (struct sockaddr*)&addr, sizeof(addr));
            if(ret < 0)
            {
                LOG(ERROR, "bind udp port failed") << std::endl;
                return -2;
            }

            LOG(INFO, "udp bind port is ") << UDP_PORT << std::endl;
            LOG(INFO, "Server init success...") << std::endl;
            return 0;
        }

        /*
         * 启动线程
         * */
        int start()
        {
            /*
             * udp应该有两类线程
             *      1.生产线程，负责接收udp数据，将udp数据放到消息池当中
             *      2.消费线程，负责从消息池当中获取消息，发送到在线用户的客户端
             * tcp
             *    针对每一个注册登录请求(tcp连接)，针对每一个tcp连接
             *    都创建一个线程，专门为该客户端处理注册和登录请求
             *    注册和登录完毕后，服务端要关闭连接，销毁线程
             *
             *    tcp是否创建线程取决于accept函数是否调用成功(阻塞)
             *
             * */

            //udp线程创建
            for(int i = 0;i < THREAD_COUNT; i++)
            {
                int ret = pthread_create(&con_tid_[i], NULL, consumeStart, (void*)this);
                if(ret < 0)
                {
                    LOG(ERROR, "start udp thread failed") << std::endl;
                    return -1;
                }

                ret = pthread_create(&pro_tid_[i], NULL, productStart, (void*)this);
                if(ret < 0)
                {
                    LOG(ERROR, "start udp thread failed") << std::endl;
                    return -1;
                }
            }

            //tcp线程创建
            
            struct sockaddr_in peer_addr;
            socklen_t peer_addrlen = sizeof(peer_addr);
            while(1)
            {
                int new_sock = accept(tcp_sock_, (struct sockaddr*)&peer_addr, &peer_addrlen);
                if(new_sock < 0)
                {
                    continue;
                }

                TcpConnect* tc = new TcpConnect();
                tc->SetSockfd(new_sock);
                tc->SetServer((void*)this);

                //正常接收到了连接
                //为客户端的注册和登录请求创建线程
                pthread_t tid;
                int ret = pthread_create(&tid, NULL, loginRegisterStart, (void*)tc);
                if(ret < 0)
                {
                    close(new_sock);
                    delete tc;
                    continue;
                }
            }
        }
    private:
        static void* consumeStart(void* arg)
        {
            pthread_detach(pthread_self());
            ChatServer* cs = (ChatServer*)arg;
            //1.从消息池当中获取数据
            //2.推送给所有的在线用户
            while(1)
            {
                cs->sendMsg();
            }
        }

        static void* productStart(void* arg)
        {
            pthread_detach(pthread_self());
            ChatServer* cs = (ChatServer*)arg;
            //1.接收udp数据
            //2.将数据发送到消息池
            while(1)
            {
                cs->recvMsg();
            }
        }
        static void* loginRegisterStart(void* arg)
        {
            /*
             * 1.分离自己，当线程退出之后，线程所占用的资源就被操作系统回收了
             * 2.接受1字节的数据，从而判断请求的类型
             *   根据不同的请求类型，调用不同的函数极性处理(注册&登录)
             * */
            pthread_detach(pthread_self());
            TcpConnect* tc = (TcpConnect*)arg;
            ChatServer* cs = (ChatServer*)(tc->GetServer());

            char ques_type = -1;
            ssize_t recv_size = recv(tc->GetSockfd(), &ques_type, 1, 0);
            if(recv_size < 0)
            {
                close(tc->GetSockfd());
                return NULL;
            }
            else if(recv_size == 0)
            {
                //等于0说明对端关闭连接，我们也关闭连接
                close(tc->GetSockfd());
                return NULL;
            }

            //对接收回来的数据进行判断
            int resp_status = -1;
            uint32_t user_id;
            switch(ques_type)
            {
                case REGISTER_RESQ:
                    {
                        //处理注册请求
                        resp_status = cs->dealRegister(tc->GetSockfd(), &user_id);
                        break;
                    }
                case LOGIN_RESQ:
                    {
                        //处理登录请求
                        resp_status = cs->dealLogin(tc->GetSockfd(), &user_id);
                        break;
                    }
            }

            //构造响应
            struct RelpyInfo ri;
            ri.resp_status_ = resp_status;
            ri.id_ = user_id;

            LOG(INFO, "resp_status is ") << StatusInfo [ri.resp_status_] << std::endl;
            LOG(INFO, "id is ") << ri.id_ << std::endl;

            int max_round_count = MAX_ROUND_COUNT;
            while(max_round_count > 0)
            {
                ssize_t send_size = send(tc->GetSockfd(), &ri, sizeof(ri), 0);
                if(send_size >= 0)
                {
                    LOG(INFO, "send reply success") << std::endl;
                    break;
                }

                LOG(WARNING, "send reply failed") << std::endl;
                max_round_count--;
            }

            //执行完毕， 关闭当前连接套接字
            close(tc->GetSockfd());
            delete tc;
            tc = NULL;
            return NULL;
        }

    private:
        //不管是注册成功还是注册失败，都需要给客户端返回一个应答
        int dealRegister(int new_sock, uint32_t* user_id)
        {
            //继续从tcp连接当中接受注册数据(接收RegistInfo)
            
            struct RegisterInfo ri;
            ssize_t recv_size = recv(new_sock, &ri, sizeof(ri), 0);
            if(recv_size < 0)
            {
                return -1;
            }
            else if(recv_size == 0)
            {
                close(new_sock);
                return -2;
            }

            //正常接收到了
            //需要将数据交给用户管理模块，进行注册，并且将用户数据进行留存
            int ret = user_manager_->dealRegister(ri.nick_name_, ri.school_, ri.password_, user_id);
            if(ret < 0)
            {
                //注册失败了
                return REGISTER_FAILED;
            }

            return REGISTER_SUCCESS;
        }

        int dealLogin(int new_sock, uint32_t* user_id)
        {
            //继续从tcp连接当中接受登录数据(LoginInfo)
            struct LoginInfo li;

            ssize_t recv_size = recv(new_sock, &li, sizeof(li), 0);
            if(recv_size < 0)
            {
                return -1;
            }
            else if(recv_size == 0)
            {
                close(new_sock);
                return -2;
            }

            *user_id = li.id_;
            
            //调用用户管理模块，判断登录请求当中的id和密码是否正确
            int ret = user_manager_->dealLogin(li.id_, li.password_);
            if(ret < 0)
            {
                return LOGIN_FAILED;
            }

            return LOGIN_SUCCESS;
        }

        int recvMsg()
        {
            /*
             * 1.接收udp数据
             * 2.判断该用户是否是登录用户
             * 3.判断该用户是否是第一次发送udp数据
             *      如果是：需要保存用户的udp地址，并且将该用户放到
             *              在线用户列表中
             *      如果不是：说明这个用户就是老用户了，之前已经保存
             *                过该用户的udp地址信息了
             * 4.将数据发送到消息池
             * */

            char buf[UDP_MAX_DATA_LEN] = {0};
            struct sockaddr_in peer_addr;
            socklen_t peer_addr_len = sizeof(peer_addr);
            ssize_t recv_size = recvfrom(udp_sock_, buf, sizeof(buf)-1, 0, (struct sockaddr*)&peer_addr,&peer_addr_len);
            if(recv_size < 0)
            {
                perror("recvfrom");
                sleep(1);
                LOG(ERROR, "recv udp msg failed") << std::endl;
                return -1;
            }

            UdpMsg um;
            std::string msg;
            msg.assign(buf, strlen(buf));

            std::cout << "recv udp msg is " << msg << std::endl;
            um.deserialize(msg);

            //需要使用user_id和用户管理模块进行验证
            //      1.先试用该user_id去map当中查找
            //      2.需要判断当前用户的状态，保存用户的udp地址信息
            
            if(user_manager_->isLogin(um.user_id_, peer_addr, peer_addr_len) < 0)
            {
                return -1;
            }

            msg_pool_->pushMsg(msg);
            return 0;
        }

        int sendMsg()
        {
            //1.从消息池当中获取消息
            //2.按照在线用户列表推送消息
            std::string msg;
            msg_pool_->popMsg(&msg);

            std::vector<UserInfo> vec;
            user_manager_->getOnlineUser(&vec);

            for(size_t i = 0; i < vec.size(); ++i)
            {
                sendUdpMsg(msg, vec[i].getAddrInfo(), vec[i].getAddrlenInfo());
                std::cout << i << " : " << msg << " ==> " << inet_ntoa(vec[i].getAddrInfo().sin_addr) << std::endl;
            }

            return 0;
        }

        int sendUdpMsg(const std::string& msg, struct sockaddr_in addr, socklen_t addr_len)
        {
            ssize_t send_size = sendto(udp_sock_, msg.c_str(), msg.size(), 0, (struct sockaddr*)&addr, addr_len);
            if(send_size < 0)
            {
                //wait...
                LOG(ERROR, "send msg failed, msg is ") << msg << "ip is " << inet_ntoa(addr.sin_addr) << "port is" << ntohs(addr.sin_port) << std::endl;
                return -1;
            }

            return 0;
        }
    private:
        int tcp_sock_;
        uint16_t tcp_port_;
        int udp_sock_;
        uint16_t udp_port_;

        UserManager* user_manager_;

        //udp线程的标识符数组
        pthread_t con_tid_[THREAD_COUNT];
        pthread_t pro_tid_[THREAD_COUNT];

        MsgPool* msg_pool_;

        UdpMsg* udp_msg_;
};
