#include "../udp.hpp"

#define CHECK_RET(p) if(p < 0){return -1;}

int main()
{
  UdpApi ua;
  CHECK_RET(ua.CreateSocket());
  CHECK_RET(ua.Bind("0.0.0.0", 19999));

  while(1)
  {
    std::string data;
    struct sockaddr_in peer_addr;
    socklen_t peer_len = sizeof(peer_addr);
    ua.RecvData(&data, &peer_addr, &peer_len);
    printf("cli say# %s\n", data.c_str());

    data.clear();
    printf("please enter resp# ");
    fflush(stdout);
    std::cin >> data;

    ua.SendData(data, &peer_addr, peer_len);
  }
  ua.Close();
  return 0;
}



#if 0
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>

int main()
{
  //1.创建套接字
  int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(sockfd < 0)
  {
    perror("socket error!");
    return -1;
  }
  //2.绑定端口
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(19999);
  addr.sin_addr.s_addr = inet_addr("0.0.0.0");

  int ret = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
  if(ret < 0)
  {
    perror("bind error!");
    return -1;
  }
  //3.接收
  //4.处理
  //5.回复应答
  while(1)
  {
    char buf[1024] = {0};
    struct sockaddr_in peer_addr;
    socklen_t peer_len = sizeof(peer_addr);
    ssize_t recv_size = recvfrom(sockfd, buf, sizeof(buf)-1, 0, (struct sockaddr*)&peer_addr, &peer_len);
    if(recv_size < 0)
    {
      perror("recvfrom error!");
      return -1;
    }

    printf("cli say# %s\n", buf);
    memset(buf, '\0', sizeof(buf));
    printf("please enter resp: ");
    fflush(stdout);
    std::cin >> buf;
    ssize_t send_size = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&peer_addr, peer_len);
    if(send_size < 0)
    {
      perror("sendto error!");
      return -1;
    }
  }
  close(sockfd);
  return 0;
}
#endif
