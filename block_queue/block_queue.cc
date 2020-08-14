#include <iostream>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include "block_queue.hpp"

using namespace std;

void *consumer(void* arg)
{
  BlockQueue *bq = (BlockQueue*)arg;

  int data;
  while(true)
  {
    bq->PopData(data);
    cout << "Consumer data is : " << data << endl;
    sleep(1);
  }
}

void *productor(void *arg)
{
  BlockQueue *bq = (BlockQueue*)arg;
  srand((unsigned long)time(nullptr));

  while(true)
  {
    int data = rand()%100 + 1 ;
    bq->PushData(data);
    cout << "Productor data is : " << data << endl;
  }
}

int main()
{
  BlockQueue *bq = new BlockQueue();
  pthread_t c,p;
  pthread_create(&c,nullptr,consumer,(void*)bq);
  pthread_create(&p,nullptr,productor,(void*)bq);


  pthread_join(c,nullptr);
  pthread_join(p,nullptr);

  delete bq;

  return 0;
}
