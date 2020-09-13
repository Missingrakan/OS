#pragma once

#include <iostream>
#include <string>
#include <pthread.h>
#include "Protocol.hpp"

class Task{
  private:
    int sock;
  public:
    Task(){}
    Task(int _sock):sock(_sock){}
    void Run()
    {
      std::cout << "Task ID: " << sock << " handler thread is : " << pthread_self() << std::endl;
      std::string message;
      while(1){
        if(!Sock::Recv(sock,message)){
          break;
        }
        std::cout << "消息# " << message << std::endl;
        Sock::Send(sock,message);
      }
      close(sock);
    }
    ~Task()
    {}
};
