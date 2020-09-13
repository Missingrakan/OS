#pragma once

#include <iostream>
#include "Protocol.hpp"
#include "ThreadPool.hpp"

class Server{
  private:
    int listen_sock;
    int port;
    std::string ip;
    ThreadPool tp;

  public:
    Server(std::string _ip, int _port):ip(_ip),port(_port),listen_sock(-1),tp(10)
    {}
    void InitServer()
    {
      listen_sock = Sock::Socket();
      Sock::Bind(listen_sock,ip,port);
      Sock::Listen(listen_sock,BACK_LOG);
      tp.InitThreadPool();
    }
    void Start()
    {
      for(;;)
      {
        std::string message;
        int sock = Sock::Accept(listen_sock);
        if(sock >= 0){
          Task t(sock);
          tp.PushTask(t);
        }
      }
    }
    ~Server()
    {
      if(listen_sock < 0){
        close(listen_sock);
        listen_sock = -1;
      }
    }
};

