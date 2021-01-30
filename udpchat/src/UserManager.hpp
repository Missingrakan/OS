#pragma once

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
    }

    ~UserInfo()
    {}

    std::string& getPassword()
    {
        return password_;
    }

    private:
        std::string nick_name_;
        std::string school_;
        std::string password_;

        //用户id
        uint32_t user_id_;
};

class UserManager
{
    public:
    UserManager()
    {
        user_map_.clear();
        pthread_mutex_init(&map_lock_, NULL);
        prepare_id_ = 0;
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
            pthread_mutex_unlock(&map_lock_);
            return -3;
        }

        //密码一致
        pthread_mutex_unlock(&map_lock_);
        return 0;
    }
    private:
        std::unordered_map<uint32_t, UserInfo> user_map_;
        pthread_mutex_t map_lock_;

        //预分配的用户id，当用户管理模块接受到注册请求后，将prepare_id分配给注册的用户，分配完毕后需要对prepare_id进行更新
        uint32_t prepare_id_;
};
