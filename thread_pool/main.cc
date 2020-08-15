#include "thread_pool.hpp"

int main()
{
  ThreadPool *tp = new ThreadPool(3);
  tp->InitThreadPool();
  srand((unsigned long)time(nullptr));

  std::string s = "+-*/";
  while(true)
  {
    int x = rand()%520 + 1;
    int y = rand()%50 + 50;
    char op = s[rand()%4];

    Task t(x,y,op);
    tp->PushTask(t);
  }
  delete tp;
}

