#pragma once

/*
 * 1.针对不同的用户注册，创建不同的玩家对象
 * 2.管理不同的玩家对象
 * */

#include <iostream>
#include <string>
#include <unordered_map>
#include <pthread.h>
#include "log.hpp"

using namespace std;

#define PREPAREID 1000

typedef enum PlayerStatus
{
    /*
     * 枚举玩家状态：不在线，在线，正在匹配，正在游戏
     * */

    OFFLINE=0,
    ONLINE,
    MATCHING,
    PLAYING
}status_t;

class Player
{
    public:
        Player()
        {}

        Player(const string name, const string passwd, uint32_t id)
            : name_(name), passwd_(passwd), id_(id)
        {
            win_count_ = 0;
            lose_count_ = 0;
            tie_count_ = 0;

            player_status_ = OFFLINE;

            room_id_ = 0;
        }

        ~Player()
        {}

        string& Getpasswd()
        {
            return passwd_;
        }

        void SetPlayerStatus(status_t status)
        {
            player_status_ = status;
        }

        int GetRate()
        {
            int total = win_count_ + lose_count_;

            if(total == 0)
                return 0;
            
            //将胜率由小数转换为整数
            return win_count_*100 / total;
        }

        int GetPlayerStatus()
        {
            return player_status_;
        }

        void SetRoomID(uint32_t room_id)
        {
            room_id_ = room_id;
        }

        uint32_t GetRoomID()
        {
            return room_id_;
        }

    private:
        string name_;
        string passwd_;
        uint32_t id_;

        /*
         * 保存玩家的胜利场数、失败场数、平局场数
         * */
        int win_count_;
        int lose_count_;
        int tie_count_;

        status_t player_status_;

        uint32_t room_id_;
};

class PlayerManager
{
    public:
        PlayerManager()
        {
            player_map_.clear();
            pthread_mutex_init(&map_lock_, NULL);

            prepare_id_ = PREPAREID;
        }

        ~PlayerManager()
        {
            pthread_mutex_destroy(&map_lock_);
        }

        uint32_t InsertPlayerToMap(const string& name, const string& passwd)
        {
            //使用传递的参数创建用户对象，返回用户id
            
            pthread_mutex_lock(&map_lock_);
            uint32_t id = prepare_id_++;
            Player p(name, passwd, id);
            player_map_.insert({id, p});
            pthread_mutex_unlock(&map_lock_);

            return id;
        }

        uint32_t PlayerLogin(const uint32_t id, const string& passwd)
        {
            /*
             * 完成如下功能
             * 1.通过用户id查找用户是否在map中
             *      在，进行密码校验
             *      不在，直接返回
             * 2.校验用户登录棉麻
             * 3.用户状态信息变更
             * */

            pthread_mutex_lock(&map_lock_);

            auto iter = player_map_.find(id);
            if(iter == player_map_.end())
            {
                LOG(WARNING, "id = ") << id << ", Player does not exist" << endl;
                pthread_mutex_unlock(&map_lock_);
                return 1;
            }

            if(iter->second.Getpasswd() != passwd)
            {
                LOG(WARNING, "password inconsistency") << endl;
                pthread_mutex_unlock(&map_lock_);
                return 2;
            }

            iter->second.SetPlayerStatus(ONLINE);

            pthread_mutex_unlock(&map_lock_);

            return id;
        }

        void SetPlayerStatus(const uint32_t id, status_t status)
        {
            player_map_[id].SetPlayerStatus(status);
        }

        int GetPlayerRate(const uint32_t id)
        {
            return player_map_[id].GetRate();
        }

        int GetPlayerStatus(const uint32_t id)
        {
            return player_map_[id].GetPlayerStatus();
        }

        void SetRoomID(const uint32_t id, const uint32_t room_id)
        {
            player_map_[id].SetRoomID(room_id);
        }

        uint32_t GetRoomID(const uint32_t id)
        {
            return player_map_[id].GetRoomID();
        }
    private:
        /*
         * 使用 key-value 来管理玩家对象
         * key：用户id
         * value：用户对象
         * */

        unordered_map<uint32_t, Player> player_map_;
        pthread_mutex_t map_lock_;

        //预分配用户id，当用户注册的时候，使用该变量中的id进行分配
        uint32_t prepare_id_;

};
