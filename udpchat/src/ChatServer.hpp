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


#define TCP_PORT 17878

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
        }

        ~ChatServer()
        {}

        /*
         * 初始化变量(服务)的接口，被调用者调用的接口
         * 用户管理模块的实例化对象，消息池的实例化对象
         * */
        int initServer(uint16_t tcp_port = TCP_PORT)
        {
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
            
            switch(ques_type)
            {
                case REGISTER_RESQ:
                    {
                        //处理注册请求
                        cs->dealRegister(tc->GetSockfd(), cs);
                        break;
                    }
                case LOGIN_RESQ:
                    {
                        //处理登录请求
                        break;
                    }
            }
        }

        int dealRegister(int new_sock, ChatServer* cs)
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
            cs->user_manager_->dealRegister(ri.nick_name_, ri.school_, ri.password_);
        }

        int dealLogin()
        {

        }
    private:
        int tcp_sock_;
        uint16_t tcp_port_;
        int udp_sock_;

        UserManager* user_manager_;
};
