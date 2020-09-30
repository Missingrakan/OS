#pragma once

#include <iostream>
#include <pthread.h>
#include "Protocol.hpp"
#include "Sock.hpp"
#include "Log.hpp"
#include "ThreadPool.hpp"

#define DefaultPort 8080

using namespace std;

class HttpServer{
  private:
    int listen_sock;
    int port;

    ThreadPool *tp;
  public:
    HttpServer(int _port = DefaultPort) : port(_port),listen_sock(-1)
    {}
    void InitServer(){
      //cout << port << endl;
      listen_sock = Sock::Socket();
      Sock::Bind(listen_sock,port);
      Sock::Listen(listen_sock);
      signal(SIGPIPE, SIG_IGN);

      tp = new ThreadPool();
      tp->InitThreadPool();
    }

    void Start()
    {
      LOG(Normal,"server Start success!");
      for(;;){
        int sock = Sock::Accept(listen_sock);
        if(sock >= 0){
         LOG(Normal,"get a new linking!");
         Task t;
         t.SetTask(sock,Entry::HanderRequest);
         tp->PushTask(t);
         //pthread_t tid;
         //int *p = new int(sock);
         //pthread_create(&tid, nullptr, Entry::HanderRequest, (void*)p);
         //pthread_detach(tid);
        }
      }
    }
    ~HttpServer()
    {
      if(listen_sock >= 0){
        close(listen_sock);
      }
    }
};
