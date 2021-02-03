#pragma once

#include <stdio.h>
#include <string.h>

#include <iostream>

#include <jsoncpp/json/json.h>

#define TCP_PORT 17878
#define UDP_PORT 17878

#define UDP_MAX_DATA_LEN 10240

//注册请求的数据格式
/*
 * 1.昵称
 * 2.学校
 * 3.用户密码
 *
 * client to server 时，严格按照Register格式传输注册数据
 * */
struct RegisterInfo
{
    RegisterInfo()
    {
        memset(nick_name_, '\0', sizeof(nick_name_));
        memset(school_, '\0', sizeof(school_));
        memset(password_, '\0', sizeof(password_));
    }

    char nick_name_[20];
    char school_[20];
    char password_[20];
};

//登录请求的数据格式
/*
 * 1.用户id
 * 2.密码
 * */
struct LoginInfo
{
    LoginInfo()
    {
        memset(password_, '\0', sizeof(password_));
    }

    uint32_t id_;
    char password_[20];
};

//服务端给客户端回复应答的数据格式

/*
 * 1.当前的状态(注册状态， 登录状态)
 * 2.返回用户ID，类似于注册完毕之后返回的qq号
 * */
struct RelpyInfo
{
    int resp_status_;
    uint32_t id_;
};

//枚举状态
enum Status
{
    REGISTER_FAILED = 0,
    REGISTER_SUCCESS,
    LOGIN_FAILED,
    LOGIN_SUCCESS,
    ONLINE
};

const char* StatusInfo[] =
{
    "REGISTER_FAILED",
    "REGISTER_SUCCESS",
    "LOGIN_FAILED",
    "LOGIN_SUCCESS"
};

//如何标识当前的请求是注册请求还是登录请求
/*
 * 方案：
 *      对于每一种请求，在tcp层面发送两个包
 *      第一个tcp包，发送一个字节，标识请求是"注册"还是"登录"
 *      第二个tcp包，发送具体的"注册"或者"登录"的请求数据
 * */
enum ResqType
{
    REGISTER_RESQ = 0,
    LOGIN_RESQ
};

const char* ResqTypeInfo[] =
{
    "REGISTER_RESQ",
    "LOGIN_RESQ"
};

//双方约定的udp数据格式

class UdpMsg
{
    public:
        UdpMsg()
        {}

        ~UdpMsg()
        {}

        //序列化接口，将对象转换为二进制
        void serialize(std::string* msg)
        {
            Json::Value json_msg;

            json_msg["nick_name"] = nick_name_;
            json_msg["school"] = school_;
            json_msg["user_id"] = user_id_;
            json_msg["msg"] = msg_;

            Json::FastWriter writer;
            *msg = writer.write(json_msg);
        }

        //反序列化接口，将二进制转换为对象
        void deserialize(std::string msg)
        {
            Json::Reader reader;
            Json::Value val;
            reader.parse(msg, val);

            nick_name_ = val["nick_name"].asString();
            school_ = val["school"].asString();
            user_id_ = val["user_id"].asUInt();
            msg_ = val["msg"].asString();
        }

    public:
        std::string nick_name_;
        std::string school_;
        uint32_t user_id_;
        std::string msg_;
};
