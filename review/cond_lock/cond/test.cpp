#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

//0 表示没有面
//1 表示有面
int g_bowl = 0;

pthread_mutex_t g_tex;
pthread_cond_t cond;

void* EatNoddle(void* arg)
{
    pthread_detach(pthread_self());
    while(1)
    {
        pthread_mutex_lock(&g_tex);
        if(g_bowl <= 0)
        {
            pthread_cond_wait(&cond, &g_tex);
        }
        g_bowl--;

        printf("Eat------g_bowl:%d\n", g_bowl);
        pthread_mutex_unlock(&g_tex);
        pthread_cond_signal(&cond);
    }

    return NULL;
}

void* MakeNoddle(void* arg)
{
    pthread_detach(pthread_self());
    while(1)
    {
        pthread_mutex_lock(&g_tex);
        if(g_bowl > 0)
        {
            pthread_cond_wait(&cond, &g_tex);
        }
        g_bowl++;
        printf("Make------g_bowl:%d\n", g_bowl);
        pthread_mutex_unlock(&g_tex);
        pthread_cond_signal(&cond);
    }

    return NULL;
}

int main()
{
    pthread_mutex_init(&g_tex, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_t tid;
    int ret = pthread_create(&tid, NULL, EatNoddle, NULL);
    if(ret < 0)
    {
        perror("pthread_create error!");
        return -1;
    }

    ret = pthread_create(&tid, NULL, MakeNoddle, NULL);
    if(ret < 0)
    {
        perror("pthread_create error!");
        return -1;
    }

    while(1)
    {
        sleep(1);
    }

    pthread_mutex_destroy(&g_tex);
    pthread_cond_destroy(&cond);

    return 0;
}
