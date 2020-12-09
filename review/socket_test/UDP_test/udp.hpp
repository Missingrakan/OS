#pragma once
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <string>
#include <string.h>

class UdpApi
{
  public:
    UdpApi()
    {
      _sockfd = -1;
    }

    int CreateSocket()
    {
      _sockfd = socket(AF_INET,SOCK_DGRAM, 17);
      if(_sockfd < 0)
      {
        perror("socked error!");
        return -1;
      }
      return 0;
    }

    int Bind(std::string ip, uint16_t port)
    {
      struct sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_port = htons(port);
      addr.sin_addr.s_addr = inet_addr(ip.c_str());

      int ret = bind(_sockfd, (struct sockaddr*)&addr, sizeof(addr));
      if(ret < 0)
      {
        perror("bind error!");
        return -1;
      }
      return 0;
    }

    int SendData(std::string data, struct sockaddr_in* addr, socklen_t addrlen)
    {
      ssize_t send_size = sendto(_sockfd, data.c_str(), data.size(), 0, (struct sockaddr*)addr, addrlen);
      if(send_size < 0)
      {
        perror("sendto error!");
        return -1;
      }
      return send_size;
    }

    int RecvData(std::string* data, struct sockaddr_in* addr, socklen_t* addrlen)
    {
      char buf[1024] = {0};
      ssize_t recv_size = recvfrom(_sockfd, buf, sizeof(buf)-1, 0, (struct sockaddr*)addr, addrlen);
      if(recv_size < 0)
      {
        perror("recvfrom error");
        return -1;
      }

      data->assign(buf, strlen(buf));
      return recv_size;
    }

    void Close()
    {
      close(_sockfd);
    }
  private:
    int _sockfd;
};
