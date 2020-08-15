#include "RingQueue.hpp"

void *consumer(void *arg)
{
  RingQueue *rq = (RingQueue*)arg;
  while(true)
  {
    int data = 0;
    rq->Popdata(data);
    std::cout << "consumer data done : " << data << std::endl;
  }
}

void *productor(void *arg)
{
  RingQueue *rq = (RingQueue*)arg;

  srand((unsigned long)time(nullptr));
  while(true)
  {
    int data = rand()%100 + 1;
    rq->PushData(data);
    std::cout << "product data done : " << data << std::endl;
  }
}

int main()
{
  RingQueue *rq = new RingQueue(5);

  pthread_t c,p;
  pthread_create(&c,nullptr,consumer,rq);
  pthread_create(&p,nullptr,productor,rq);

  pthread_join(c,nullptr);
  pthread_join(p,nullptr);

  delete rq;
  return 0;
}
