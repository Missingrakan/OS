#pragma once

#include <iostream>
#include <vector>
#include <unistd.h>
#include <semaphore.h>

class RingQueue
{
  private:
    std::vector<int> ring;
    int cap;
    sem_t sem_blank;
    sem_t sem_data;

    int product_step;
    int consume_step;
  private:
    void P(sem_t &sem)
    {
      sem_wait(&sem);
    }
    void V(sem_t &sem)
    {
      sem_post(&sem);
    }
  public:
    RingQueue(int _cap = 20) : cap(_cap),ring(_cap)
    {
      product_step = consume_step = 0;
      sem_init(&sem_blank,0,_cap);
      sem_init(&sem_data,0,0);
    }
    void PushData(int &data)
    {
      P(sem_blank);
      //product data
      ring[product_step] = data;
      V(sem_data);
      product_step++;
      product_step %= cap;
    }
    void Popdata(int &data)
    {
      sleep(1);
      P(sem_data);
      //consume data
      data = ring[consume_step];
      V(sem_blank);
      consume_step++;
      consume_step %= cap;
    }
    ~RingQueue()
    {
      sem_destroy(&sem_blank);
      sem_destroy(&sem_data);
    }
};
