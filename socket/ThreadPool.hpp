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
      std::cout << "thread " << pthread_self() << " wait..." << std::endl;
      pthread_cond_wait(&cond,&lock);
    }
    void WakeUp()
    {
      std::cout << "thread " << pthread_self() << " wakeup,handler task..." << std::endl;
      pthread_cond_signal(&cond);
    }
    void PopTask(Task &t)
    {
      t = q.front();
      q.pop();
    }
    void PushTask(Task &t)
    {
      LockQueue();
      q.push(t);
      UnlockQueue();
      WakeUp();
    }
    static void *HandlerTask(void *arg)
    {
      pthread_detach(pthread_self());
      ThreadPool *tp = (ThreadPool*)arg;
      for(;;)
      {
        Task t;
        tp->LockQueue();
        while(tp->IsEmpty()){
          tp->ThreadWait();
        }
        tp->PopTask(t);
        tp->UnlockQueue();
        t.Run();
      }
    }
    void InitThreadPool()
    {
      for(auto i = 0; i < num; i++){
        pthread_t tid;
        pthread_create(&tid,nullptr,HandlerTask,(void*)this);
      }
    }
    ~ThreadPool()
    {
      pthread_mutex_destroy(&lock);
      pthread_cond_destroy(&cond);
    }
};
