#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <vector>

#define CAPACITY 4
#define THREADCOUNT 2

class RingQueue
{
    public:
        RingQueue()
            : vec_(CAPACITY)
        {
            capacity_ = CAPACITY;

            //位置
            pos_read_ = 0;
            pos_write_ = 0;

            sem_init(&lock_, 0, 1);
            sem_init(&read_, 0, 0);
            sem_init(&write_, 0, CAPACITY);
        }

        ~RingQueue()
        {
            sem_destroy(&read_);
            sem_destroy(&write_);
        }
        
        void Push(int data)
        {
            sem_wait(&write_);

            sem_wait(&lock_);
            vec_[pos_write_] = data;
            pos_write_ = (pos_write_ + 1) % capacity_;
            sem_post(&lock_);

            sem_post(&read_);
        }

        void pop(int *data)
        {
            sem_wait(&read_);

            sem_wait(&lock_);
            *data = vec_[pos_read_];
            pos_read_ = (pos_read_ + 1) % capacity_;
            sem_post(&lock_);

            sem_post(&write_);
        }
    private:
        std::vector<int> vec_;
        int capacity_;

        int pos_read_;
        int pos_write_;

        sem_t lock_;
        sem_t read_;
        sem_t write_;
};

void* ReadStart(void* arg)
{
    RingQueue* rq = (RingQueue*)arg;
    while(1)
    {
        int data;
        rq->pop(&data);
        printf("i am %p, i read %d\n", pthread_self(), data);
    }
    return NULL;
}

void* WriteStart(void* arg)
{
    RingQueue* rq = (RingQueue*)arg;
    int data = 0;
    while(1)
    {
        rq->Push(data);
        printf("i am %p, i write %d\n", pthread_self(), data);
        data++;
    }
    return NULL;
}

int main()
{
    pthread_t rd[THREADCOUNT], wr[THREADCOUNT];
    RingQueue* rq = new RingQueue();
    for(int i = 0; i < THREADCOUNT; i++)
    {
        int ret = pthread_create(&rd[i], NULL, ReadStart, (void*)rq);
        if(ret < 0)
        {
            perror("pthread_create error!");
            return -1;
        }

        ret = pthread_create(&wr[i], NULL, WriteStart, (void*)rq);
        if(ret < 0)
        {
            perror("pthread_create errro!");
            return -1;
        }
    }

    for(int i = 0; i < THREADCOUNT; ++i)
    {
        pthread_join(rd[i], NULL);
        pthread_join(wr[i], NULL);
    }

    delete rq;
    return 0;
}
