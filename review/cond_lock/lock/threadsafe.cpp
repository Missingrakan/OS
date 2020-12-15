#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define THREAD_COUNT 4

int g_tickets = 100;

pthread_mutex_t tex;

struct ThreadData
{
    int num_;

    ThreadData()
    {
        num_  = 1;
    }
};

void* ThreadStart(void* arg)
{
    pthread_detach(pthread_self());
    ThreadData* td = (ThreadData*)arg;
    while(1)
    {
        pthread_mutex_lock(&tex);
        if(g_tickets > 0)
        {
            printf("i am %p, i have tickets is %d\n", pthread_self(), g_tickets);
            g_tickets--;
        }
        else
        {
            pthread_mutex_unlock(&tex);
            break;
        }
        pthread_mutex_unlock(&tex);
    }

    delete td;
    return NULL;
}

int main()
{
    pthread_mutex_init(&tex, NULL);
    pthread_t tid[THREAD_COUNT];
    for(int i = 0; i < THREAD_COUNT; ++i)
    {
        struct ThreadData* td = new ThreadData;
        td->num_ = 1;
        int ret = pthread_create(&tid[i], NULL, ThreadStart, (void*)td);
        if(ret < 0)
        {
            perror("pthread_create error!");
            return -1;
        }
    }

    while(1)
    {
        sleep(1);
    }

    pthread_mutex_destroy(&tex);

    return 0;
}
