#include "../udp.hpp"

#define CHECK_RET(p) if(p < 0){return -1;}

int main(int argc, char* argv[])
{
  if(argc != 5)
  {
    printf("./udp_cli -ip [svr_ip] -port [svr_port]\n");
    return -1;
  }

  std::string svr_ip;
  uint16_t svr_port;
  for(int i = 0; i < argc; i++)
  {
    if(strcmp(argv[i], "-ip") == 0)
      svr_ip = argv[i+1];
    else if(strcmp(argv[i], "-port") == 0)
      svr_port = atoi(argv[i+1]);
  }

  UdpApi ua;
  CHECK_RET(ua.CreateSocket());

  while(1)
  {
    struct sockaddr_in svr_addr;
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(svr_port);
    svr_addr.sin_addr.s_addr = inet_addr(svr_ip.c_str());

    std::string data;
    printf("please input msg# ");
    fflush(stdout);
    std::cin >> data;
    ua.SendData(data, &svr_addr, sizeof(svr_addr));

    data.clear();

    ua.RecvData(&data, NULL, NULL);
    printf("svr say# %s\n", data.c_str());
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

  //发送数据
  //接收应答
  while(1)
  {
    char buf[1024] = {0};
    memset(buf, '\0', sizeof(buf));
    struct sockaddr_in svr_addr;
    std::cin >> buf;
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(19999);
    svr_addr.sin_addr.s_addr = inet_addr("192.168.59.131");

    ssize_t send_size = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&svr_addr, sizeof(svr_addr));
    if(send_size < 0)
    {
      perror("sendto error!");
      return -1;
    }

    memset(buf, '\0', sizeof(buf));
    ssize_t recv_size = recvfrom(sockfd, buf, sizeof(buf)-1, 0, NULL, NULL);
    if(recv_size < 0)
    {
      perror("recvfrom error!");
      return -1;
    }

    printf("svr say# %s\n", buf);
  }
  close(sockfd);
  return 0;
}
#endif
