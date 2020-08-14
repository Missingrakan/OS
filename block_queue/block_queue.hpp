#pragma once

#include <iostream>
#include <queue>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

class BlockQueue
{
  private:
    std::queue<int> q;
    int cap;

    pthread_mutex_t lock;
    pthread_cond_t cond_c;  //提供给消费者的条件变量
    pthread_cond_t cond_p;  //提供给生产者的条件变量
  private:
    bool IsFull()
    {
      return cap == q.size();
    }
    bool IsEmpty()
    {
      return q.size() == 0;
    }
    void LockQueue()
    {
      pthread_mutex_lock(&lock);
    }
    void UnlockQueue()
    {
      pthread_mutex_unlock(&lock);
    }
    void ProductorWait()
    {
      pthread_cond_wait(&cond_p,&lock);
      //1.自动释放！且是原子的。2.当线程在cond下被唤醒的时候，会自动重新获取对应的锁。
    }
    void ConsumerWait()
    {
      pthread_cond_wait(&cond_c,&lock);
    }
  public:
    BlockQueue(int _cap = 5):cap(_cap)
    {
      pthread_mutex_init(&lock,nullptr);
      pthread_cond_init(&cond_c,nullptr);
      pthread_cond_init(&cond_p,nullptr);
    }
    //生产者
    void PushData(int &data)
    {
      LockQueue();
      //lock，决定哪个生产者进行生产
      while(IsFull())
      {
        //阻塞等待，等待队列有空间
        ProductorWait();
      }
      if(q.size() > cap / 2)
      {
        pthread_cond_signal(&cond_c);
      }
      q.push(data);
      UnlockQueue();
      pthread_cond_signal(&cond_c);
    }
    //消费者
    void PopData(int &data)
    {
      LockQueue();
      //lock,决定让哪个消费者进行消费
      while(IsEmpty())
      {
        ConsumerWait();
      }
      data = q.front();
      q.pop();
      UnlockQueue();
      pthread_cond_signal(&cond_p);
    }
    ~BlockQueue()
    {
      pthread_mutex_destroy(&lock);
      pthread_cond_destroy(&cond_c);
      pthread_cond_destroy(&cond_p);
    }
};
