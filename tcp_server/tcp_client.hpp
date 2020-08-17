#pragma once

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class Client
{
  private:
    int sockfd;
    std::string svr_ip;
    int svr_port;

  public:
    Client(std::string _ip, int _port):svr_ip(_ip),svr_port(_port),sockfd(-1)
    {}
    void InitClient()
    {
      sockfd = socket(AF_INET,SOCK_STREAM,0);
      if(sockfd < 0){
        std::cerr << "socket error..." << std::endl;
        exit(2);
      }
      std::cout << "Init client success ..." << std::endl;
    }
    void Connect()
    {
      struct sockaddr_in svr;
      svr.sin_family = AF_INET;
      svr.sin_port = htons(svr_port);
      svr.sin_addr.s_addr = inet_addr(svr_ip.c_str());

      if(connect(sockfd,(struct sockaddr*)&svr,sizeof(svr)) == 0){
        std::cout << "connect success ..." << std::endl;
      }
      else{
        std::cout << "connect failed ..." << std::endl;
        exit(3);
      }
    }
    void Start()
    {
      std::string message;
      char buf[1024];
      while(1){
        std::cout << "Please Enter Your Message# ";
        std::cin >> message;
        write(sockfd,message.c_str(),message.size());
        if(message == "q" || message == "quit"){
          break;
        }

        ssize_t s = read(sockfd,buf,sizeof(buf)-1);
        if(s > 0){
          buf[s] = 0;
          std::cout << "Server Echo# " << buf << std::endl;
        }
      }
      std::cout << "quit!" << std::endl;

    }
    ~Client()
    {
      if(sockfd >= 0){
        close(sockfd);
      }
    }
};
