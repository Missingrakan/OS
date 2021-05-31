#pragma once

#include <vector>

#include "player.hpp"
#include "room.hpp"

#define VEC_SIZE 101

/*
 * 这个文件用来进行全局管理，玩家管理模块（注册+登录）、玩家匹配、房间管理
 * */

class Hall
{
    public:
        Hall()
            : pm_(nullptr), match_pool_(VEC_SIZE), match_pool_num_(0), rm_(nullptr)
        {
            pthread_mutex_init(&vec_lock_, nullptr);
            pthread_cond_init(&vec_cond_, nullptr);
        }

        ~Hall()
        {
            if(pm_)
            {
                delete pm_;
                pm_ = nullptr;
            }

            if(rm_)
            {
                delete rm_;
                rm_ = nullptr;
            }

            pthread_mutex_destroy(&vec_lock_);
            pthread_cond_destroy(&vec_cond_);
        }

        int InitHall()
        {
            pm_ = new PlayerManager();
            if(pm_ == nullptr)
            {
                return -1;
            }

            LOG(INFO, "The PlayerManager segment start up success...") << endl;

            rm_ = new RoomManager();
            if(rm_ == nullptr)
            {
                return -1;
            }

            LOG(INFO, "The RoomManager segment start up success...") << endl;

            pthread_t tid;
            int ret = pthread_create(&tid, NULL, MatchServer, (void*)this);
            if(ret < 0)
            {
                LOG(ERROR, "pthread create error!") << endl;
                return -1;
            }

            LOG(INFO, "The Match Thread start up success...") << endl;

            return 0;
        }

        uint32_t PlayerRegister(const string& name, const string& passwd)
        {
            return pm_->InsertPlayerToMap(name, passwd);
        }

        uint32_t PlayerLogin(const uint32_t id, const string& passwd)
        {
            return pm_->PlayerLogin(id, passwd);
        }

        bool PushPlayerToMatchPool(const uint32_t id)
        {
            /*
             * 函数要做的事
             * 1.改变用户状态
             * 2.通过id查找用户胜率
             * 3.将用户插入到对应胜率的vector中
             * 4.考虑加锁和通知机制
             * */
        
            LOG(INFO, "push ") << id << " to match pool..." << endl;
            pm_->SetPlayerStatus(id, MATCHING);
            int rate = pm_->GetPlayerRate(id);

            if(rate < 0 || rate > 100)
                return false;

            pthread_mutex_lock(&vec_lock_);
            
            auto &v = match_pool_[rate];
            for(auto it = v.begin(); it != v.end(); ++it)
            {
                if(*it == id)
                {
                    pthread_mutex_unlock(&vec_lock_);
                    return true;
                }
            }
            v.push_back(id);
            match_pool_num_++;

            pthread_mutex_unlock(&vec_lock_);
            pthread_cond_signal(&vec_cond_);

            return true;
        }

        void GetMatchPlayer(vector<uint32_t>* vec)
        {
            for(int i = VEC_SIZE-1; i >= 0; --i)
            {
                auto& v = match_pool_[i];
                if(v.empty())
                    continue;

                for(auto it = v.begin(); it != v.end(); ++it)
                {
                    vec->push_back(*it);
                }
            }
        }

        void MatchPoolClear(const uint32_t id)
        {
            for(int i = VEC_SIZE-1; i >= 0; --i)
            {
                auto& v = match_pool_[i];
                if(v.empty())
                    continue;

                vector<uint32_t>().swap(v);
            }

            if(id >= 1000)
                pm_->SetPlayerStatus(id, ONLINE);

            match_pool_num_ = 0;
        }

        /* 
         * MatchServer这个函数为线程的入口函数，此函数完成从匹配池中获取相近胜率的玩家列表
         * 函数中需考虑加解锁操作
         * 1.判断匹配池当中的匹配人数
         *      < 2人，该线程等待
         *      > 2人，进行接下来的操作
         * 2.从match_pool_中获取匹配序列，（序列是按照玩家胜率排序的）
         * 3.判断序列是奇数还是偶数
         *      是偶数，直接进行两两分组
         *      是奇数，有个人肯定匹配不上，将其状态设置为在线即可
         * 4.遍历序列，两两分组
         *      分组成功的玩家更改状态，
         *      创建游戏房间进行环境准备
         *      对每一个用户设计房间号标识
         * 5.清空匹配池
         * */

        static void* MatchServer(void* arg)
        {
            pthread_detach(pthread_self());
            Hall* ha = (Hall*)arg;

            uint32_t last_id = 0;
            while(1)
            {
                pthread_mutex_lock(&ha->vec_lock_);

                while(ha->match_pool_num_ < 2)
                {
                    LOG(INFO, "matching, please wait...") << endl;
                    pthread_cond_wait(&ha->vec_cond_, &ha->vec_lock_);
                }

                vector<uint32_t> vec;
                ha->GetMatchPlayer(&vec);

                size_t vec_size = vec.size();
                if(vec_size & 1)
                {
                    //奇数个
                    last_id = vec[vec_size-1];
                    vec_size &= (~1);
                }

                for(int i = vec_size-1; i >= 0; i-=2)
                {
                    uint32_t player_one = vec[i];
                    uint32_t player_two = vec[i-1];

                    LOG(INFO, "matching success! player1：") << player_one << " vs player2：" << player_two << endl;

                    ha->pm_->SetPlayerStatus(player_one, PLAYING);
                    ha->pm_->SetPlayerStatus(player_two, PLAYING);

                    //创建房间，设置房间号 
                    uint32_t room_id = ha->rm_->CreateRoom(player_one, player_two);
                    ha->pm_->SetRoomID(player_one, room_id);
                    ha->pm_->SetRoomID(player_two, room_id);

                    LOG(INFO, "create game room ")  << room_id  << " for " << "player1: " << player_one << " and " << "player2: " << player_two << endl;
                }

                ha->MatchPoolClear(last_id);
                pthread_mutex_unlock(&ha->vec_lock_);
            }

            return NULL;
        }

        int GetPlayerStatus(const uint32_t id)
        {
            return pm_->GetPlayerStatus(id);
        }

        void PopPlayer(const uint32_t id)
        {
            MatchPoolClear(id);
        }

        uint32_t GetPlayerRoomID(const uint32_t id)
        {
            return pm_->GetRoomID(id);
        }

        char GetPlayerPiece(const uint32_t id, const uint32_t room_id)
        {
            return rm_->GetPlayerPiece(id, room_id);
        }

        string GetPlayerBoard(const uint32_t room_id)
        {
            return rm_->GetPlayerBoard(room_id);
        }

        bool IsMyTurn(const uint32_t id, const uint32_t room_id)
        {
            return rm_->IsMyTurn(id, room_id);
        }

        char Step(const uint32_t id, const uint32_t room_id, const int x, const int y)
        {
            return rm_->Step(id, room_id, x, y);
        }

        char GetResult(const uint32_t room_id)
        {
            return rm_->GetResult(room_id);
        }
    private:
        PlayerManager* pm_;
        vector<vector<uint32_t> > match_pool_;
        int match_pool_num_;

        pthread_mutex_t vec_lock_;
        pthread_cond_t vec_cond_;

        RoomManager* rm_;
};
