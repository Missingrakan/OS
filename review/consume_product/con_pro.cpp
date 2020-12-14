#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <queue>

#define CAPACITY 5
#define THREAD_COUNT 2

pthread_mutex_t g_mutex;

//定义线程安全的队列
class SafeQueue
{
    public:
        SafeQueue()
        {
            capacity_ = CAPACITY;
            pthread_mutex_init(&mutex_, NULL);
            pthread_cond_init(&product_cond_, NULL);
            pthread_cond_init(&consume_cond_, NULL);
        }

        ~SafeQueue()
        {
            pthread_mutex_destroy(&mutex_);
            pthread_cond_destroy(&product_cond_);
            pthread_cond_destroy(&consume_cond_);
        }

    public:
        void Push(int data)
        {
            pthread_mutex_lock(&mutex_);
            while(queue_.size() >= capacity_)
            {
                pthread_cond_wait(&product_cond_, &mutex_);
            }
            queue_.push(data);
            pthread_mutex_unlock(&mutex_);
            pthread_cond_signal(&consume_cond_);
        }

        void Pop(int* data)
        {
            pthread_mutex_lock(&mutex_);
            while(queue_.empty())
            {
                pthread_cond_wait(&consume_cond_, &mutex_);
            }
            *data = queue_.front();
            queue_.pop();
            pthread_mutex_unlock(&mutex_);
            pthread_cond_signal(&product_cond_);
        }
    private:
        std::queue<int> queue_;
        size_t capacity_;

        pthread_mutex_t mutex_;
        pthread_cond_t consume_cond_;
        pthread_cond_t product_cond_;
};

void* consume_start(void* arg)
{
    SafeQueue* sq = (SafeQueue*)arg;
    while(1)
    {
        int data;
        sq->Pop(&data);
        printf("i am %p, i consume %d\n", pthread_self(), data);
    }
    return NULL;
}

int count = 0;
void* product_start(void* arg)
{
    SafeQueue* sq = (SafeQueue*)arg;
    while(1)
    {
        pthread_mutex_lock(&g_mutex);
        sq->Push(count);
        printf("i am %p, i product %d\n", pthread_self(), count);
        count++;
        pthread_mutex_unlock(&g_mutex);
    }
    return NULL;
}

int main()
{
    pthread_mutex_init(&g_mutex, NULL);
    SafeQueue* sq = new SafeQueue();

    pthread_t consume[THREAD_COUNT], product[THREAD_COUNT];
    for(int i = 0; i < THREAD_COUNT; ++i)
    {
        int ret = pthread_create(&consume[i], NULL, consume_start, (void*)sq);
        if(ret < 0)
        {
            perror("pthread_create error!");
            return -1;
        }

        ret = pthread_create(&product[i], NULL, product_start, (void*)sq);
        if(ret < 0)
        {
            perror("pthread_create error!");
            return -1;
        }
    }

    for(int i = 0; i < THREAD_COUNT; ++i)
    {
        pthread_join(product[i], NULL);
        pthread_join(consume[i], NULL);
    }
    delete sq;
    pthread_mutex_destroy(&g_mutex);

    return 0;
}

