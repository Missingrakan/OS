#pragma once

#include <iostream>
#include <queue>
#include <pthread.h>
#include "Task.hpp"

class ThreadPool{
  private:
    int num;
    std::queue<Task> q;
    pthread_mutex_t lock;
    pthread_cond_t cond;
  public:
    ThreadPool(int _num = 5):num(_num)
    {
      pthread_mutex_init(&lock,nullptr);
      pthread_cond_init(&cond,nullptr);
    }
    bool IsEmpty()
    {
      return q.empty();
    }
    void LockQueue()
    {
      pthread_mutex_lock(&lock);
    }
    void UnlockQueue()
    {
      pthread_mutex_unlock(&lock);
    }
    void ThreadWait()
    {
      std::cout << "thread " << pthread_self() << "wait..." << std::endl;
      pthread_cond_wait(&cond,&lock);
    }
};
