#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <iostream>
#include <string>
#include <unordered_map>

#include "ConnectInfo.hpp"

/*
 * key-value 结构体来保存用户信息
 * key：用户id
 * value：用来保存单个用户的信息
 * eg：1:{nickname, school, passwd}
 *     2:{nickname, school, passwd}
 *
 * 缺陷：
 *      每次重启服务进程后，之前注册过的所有用户没有了
 * 解决方案：
 *      将之前注册的数据放到数据库中进行保存
 *      1.启动服务的时候，需要去数据库中获取之前注册用户的信息，加载到进程内存中，在判断登录的时候，不需要查询数据库，用户访问就比较便捷
 *      2.注册完毕之后，还需要将数据及时写到数据当中(持久化)
 * 解决方案2：
 *      将注册的数据写入文件
 * */

class UserInfo
{
    public:
    UserInfo(const std::string& nick_name, const std::string& school, const std::string& password, uint32_t user_id)
    {
        nick_name_ = nick_name;
        school_ = school;
        password_ = password;
        user_id_ = user_id;
        user_status_ = REGISTER_FAILED;

        memset((void*)&addr_, '\0', sizeof(struct sockaddr_in));
        addr_len_ = 0;
    }

    ~UserInfo()
    {}

    std::string& getPassword()
    {
        return password_;
    }

    void setUserStatus(int status)
    {
        user_status_ = status;
    }

    int getUserStatus()
    {
        return user_status_;
    }

    void setAddrInfo(struct sockaddr_in addr)
    {
        memcpy(&addr_, &addr, sizeof(addr));
    }

    void setAddrlenInfo(socklen_t addr_len)
    {
        addr_len_ = addr_len;
    }

    struct sockaddr_in& getAddrInfo()
    {
        return addr_;
    }

    socklen_t getAddrlenInfo()
    {
        return addr_len_;
    }

    private:
        std::string nick_name_;
        std::string school_;
        std::string password_;

        //用户id
        uint32_t user_id_;

        int user_status_;

        //用户udp地址信息
        struct sockaddr_in addr_;
        socklen_t addr_len_;
};

class UserManager
{
    public:
    UserManager()
    {
        user_map_.clear();
        pthread_mutex_init(&map_lock_, NULL);
        prepare_id_ = 0;

        online_user_.clear();
    }

    ~UserManager()
    {
        pthread_mutex_destroy(&map_lock_);
    }

    //处理注册请求
    int dealRegister(const std::string& nick_name, const std::string& school, const std::string& password, uint32_t* user_id)
    {
        //1.判断字段是否为空
        if(nick_name.size() == 0 || school.size() == 0 || password.size() == 0)
        {
            return -1;
        }
        
        //2.创建单个用户UserInfo这个类的对象
        pthread_mutex_lock(&map_lock_);
        
        //3.分配用户id
        UserInfo ui(nick_name, school, password, prepare_id_);
        *user_id = prepare_id_;
        
        //更改用户状态
        ui.setUserStatus(REGISTER_SUCCESS);
        
        //4.将用户数据插入到map当中
        user_map_.insert(std::make_pair(prepare_id_, ui));
        
        //5.更新预分配用户id
        prepare_id_++;
        pthread_mutex_unlock(&map_lock_);
        return 0;
    }

    int dealLogin(uint32_t id, const std::string& password)
    {
        //1.判断password是否为空
        //2.使用id，在unordered_map当中查找是否有该id对应的值
        //      2.1 没找到该id对应的值，返回登录失败
        //      2.2 找到了id对应的值
        //          对比保存的密码值和本次提交的密码值是否一致
        //              a. 如果一致，则登录成功
        //              b. 如果不一致，则登陆失败
        
        if(password.size() == 0)
        {
            return -1;
        }

        pthread_mutex_lock(&map_lock_);
        auto it = user_map_.find(id);
        if(it == user_map_.end())
        {
            pthread_mutex_unlock(&map_lock_);
            return -2;
        }

        //找到了，比较密码
        std::string reg_password = it->second.getPassword();
        if(reg_password != password)
        {
            it->second.setUserStatus(LOGIN_FAILED);
            pthread_mutex_unlock(&map_lock_);
            return -3;
        }

        //密码一致
        it->second.setUserStatus(LOGIN_SUCCESS);
        pthread_mutex_unlock(&map_lock_);
        return 0;
    }

    /*
     * user_id : 用户id
     * addr : udp客户端的地址信息，是为了后面推送消息所保存的
     * addr_len : udp客户端的地址信息长度
     * */
    int isLogin(uint32_t user_id, struct sockaddr_in addr, socklen_t addr_len)
    {
        //1.使用user_id去mao当中查询，是否存在该用户
        //如果存在，则获取用户信息，判断用户状态
        //如果不存在，直接返回，刚刚接收的udp数据直接丢弃
        
        pthread_mutex_lock(&map_lock_);
        auto it = user_map_.find(user_id);
        if(it == user_map_.end())
        {
            pthread_mutex_unlock(&map_lock_);
            return -1;
        }

        //2.判断用户状态
        //      2.1 第一次发送，我们保存该用户的地址信息
        //      2.2 如果不是第一次发送，则不用添加地址信息
        
        if(it->second.getUserStatus() <= LOGIN_FAILED)
        {
            pthread_mutex_unlock(&map_lock_);
            return -1;
        }
        else if(it->second.getUserStatus() == LOGIN_SUCCESS)
        {
            //第一次发送udp消息(刚刚登陆)
            it->second.setUserStatus(ONLINE);
            it->second.setAddrInfo(addr);
            it->second.setAddrlenInfo(addr_len);

            //将用户信息添加到在线用户列表当中
            //本质上是为推送消息到udp客户端做铺垫
            online_user_.push_back(it->second);
        }
        pthread_mutex_unlock(&map_lock_);
        return 0;
    }

    void getOnlineUser(std::vector<UserInfo>* vec)
    {
        *vec = online_user_;
    }
    private:
        std::unordered_map<uint32_t, UserInfo> user_map_;
        pthread_mutex_t map_lock_;

        //预分配的用户id，当用户管理模块接受到注册请求后，将prepare_id分配给注册的用户，分配完毕后需要对prepare_id进行更新
        uint32_t prepare_id_;

        //保存在线用户的数组
        std::vector<UserInfo> online_user_;
};
