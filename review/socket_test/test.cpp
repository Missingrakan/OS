#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main()
{
  int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(sockfd < 0)
  {
    perror("socket error!");
    return -1;
  }

  //地址信息
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(18989);
  //1.将点分十进制IP转换为无符号的四字节整数
  //2.将该整数转换成为网络字节序
  
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  int ret = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
  if(ret < 0)
  {
    perror("bind error!");
    return -1;
  }


  while(1)
  {
    sleep(1);
  }
  return 0;
}
