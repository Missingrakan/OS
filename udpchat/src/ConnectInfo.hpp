#pragma once

#include <stdio.h>
#include <string.h>

#include <iostream>

#define TCP_PORT 17878

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
    LOGIN_SUCCESS
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
