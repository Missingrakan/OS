#pragma once

#include <iostream>
#include "Protocol.hpp"

class Client{
  private:
    int sock;
    int port;
    std::string ip;
  public:
    Client(std::string _ip, int _port):ip(_ip),port(_port),sock(-1)
    {}
    void InitClient()
    {
      sock = Sock::Socket();
      Sock::Connect(sock,ip,port);
    }
    void Start()
    {
      while(1)
      {
        std::string output;
        std::cout << "请输入# ";
        std::string input;
        std::cin >> input;

        Sock::Send(sock,input);

        if(Sock::Recv(sock,output)){
          std::cout << "服务器回显# " << output << std::endl; 
        }
        else{
          break;
        }
      }
    }
    ~Client()
    {
      if(sock < 0){
        close(sock);
        sock = -1;
      }
    }
};
