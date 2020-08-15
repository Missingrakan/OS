#pragma once

#include <iostream>
#include <queue>
#include <string>
#include <unistd.h>
#include <pthread.h>

#define NUM 5

class Task
{
  private:
    int x;
    int y;
    char op;
  public:
    Task()
    {
      x = 1;
      y = 1;
      op = '+';
    }
    Task(int _x,int _y, char _op):x(_x), y(_y), op(_op)
    {}
    ~Task()
    {}
    void Run()
    {
      int result = 0;
      switch(op)
      {
        case '+':
          result = x + y;
          break;
        case '-':
          result = x - y;
          break;
        case '*':
          result = x * y;
          break;
        case '/':
          result = (y == 0) ? -1 : (x/y);
          break;
        default:
          break;
      }
      std::cout << "thread id: " << pthread_self() << ":" << x << op << y << "=" << result << std::endl;
     // sleep(1);
    }
};
class ThreadPool
{
  private:
    int num;
    std::queue<Task> q;

    pthread_mutex_t lock;
    pthread_cond_t cond;
  public:
    bool TaskQueueEmpty()
    {
      return q.size() == 0;
    }
    void ThreadWait()
    {
      pthread_cond_wait(&cond,&lock);
    }
    void ThreadWakeUp()
    {
      pthread_cond_signal(&cond);
    }
    void LockTaskQueue()
    {
      pthread_mutex_lock(&lock);
    }
    void UnlockTaskQueue()
    {
      pthread_mutex_unlock(&lock);
    }
  public:
    ThreadPool(int _num = NUM):num(_num)
    {}
    static void*Routine(void *arg)
    {
      ThreadPool *self = (ThreadPool*)arg;
      while(true)
      {
        //判断任务队列里是否有任务？
        //如果有任务，就处理任务
        //如果没有任务，线程就进行休眠
        //休眠期间，如何知道有任务进而被唤醒呢
        //当有任务被新插入的时候
        self->LockTaskQueue();
        while(self->TaskQueueEmpty()){
          self->ThreadWait();
        }

        Task t;
        self->PopTask(t);

        self->UnlockTaskQueue();

        t.Run();
      }
    }
    void InitThreadPool()
    {
      pthread_t tid;
      for(int i = 0; i < num; ++i)
      {
        pthread_create(&tid,nullptr,Routine,this/*传入当前对象*/);
      }
      pthread_mutex_init(&lock,nullptr);
      pthread_cond_init(&cond,nullptr);
    }
    void PopTask(Task &t)
    {
      t = q.front();
      q.pop();
    }
    void PushTask(Task &t)
    {
      LockTaskQueue();
      q.push(t);
      UnlockTaskQueue();
      ThreadWakeUp();
    }
    ~ThreadPool()
    {
      pthread_mutex_destroy(&lock);
      pthread_cond_destroy(&cond);
    }
};


