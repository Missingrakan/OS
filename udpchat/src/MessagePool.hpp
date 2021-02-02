#pragma once

#include <iostream>
#include <string>
#include <queue>

#define CAPACITY 1024

class MsgPool
{
    public:
        MsgPool(size_t capa = CAPACITY)
        {
            capacity_ = capa;
            pthread_mutex_init(&lock_vec_, NULL);
            pthread_cond_init(&cond_con_, NULL);
            pthread_cond_init(&cond_pro_, NULL);
        }

        ~MsgPool()
        {
            pthread_mutex_destroy(&lock_vec_);
            pthread_cond_destroy(&cond_con_);
            pthread_cond_destroy(&cond_pro_);
        }

        void pushMsg(const std::string& msg)
        {
            pthread_mutex_lock(&lock_vec_);
            while(vec_.size() >= capacity_)
            {
                pthread_cond_wait(&cond_pro_, &lock_vec_);
            }

            vec_.push(msg);
            pthread_mutex_unlock(&lock_vec_);
            pthread_cond_signal(&cond_con_);
        }

        void popMsg(std::string* msg)
        {
            pthread_mutex_lock(&lock_vec_);
            while(vec_.empty())
            {
                pthread_cond_wait(&cond_con_, &lock_vec_);
            }
            *msg = vec_.front();
            vec_.pop();
            pthread_mutex_unlock(&lock_vec_);
            pthread_cond_signal(&cond_pro_);
        }
    private:
        /*
         * vec_ : 保存消息的容器
         * capacity_ : 定义的消息容器的容量
         * lock_vec_ : 保护消息容器的锁
         * cond_con_ : 消费者的条件变量
         * cond_pro_ : 生产者的条件变量
         * */
        std::queue<std::string> vec_;
        size_t capacity_;

        pthread_mutex_t lock_vec_;
        pthread_cond_t cond_con_;
        pthread_cond_t cond_pro_;
};
