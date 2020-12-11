#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "../ConnectInfo.h"

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd < 0)
    {
        perror("socker error!");
        return -1;
    }

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(19999);
    dest_addr.sin_addr.s_addr = inet_addr("192.168.59.131");

    int ret = connect(sockfd, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    if(ret < 0)
    {
        perror("connect error!");
        return -1;
    }

    while(1)
    {
        struct AppHeader ah;
        ah.packet_size = 4;

        //send(sockfd, &ah, sizeof(ah), 0);
        char buf[1024] = {0};
        strcpy(buf, "1+1\r\n");

        char* m = (char*)malloc(sizeof(struct AppHeader) + strlen(buf));
        memcpy(m, (struct AppHeader*)&ah, sizeof(ah));
        memcpy(m + sizeof(ah), buf, strlen(buf));
        ret = send(sockfd, buf, strlen(buf), 0);
        if(ret < 0)
        {
            perror("send error!");
            return -1;
        }

        memset(buf, '\0', sizeof(buf));
        strcpy(buf, "2+1");
        ret = send(sockfd, buf, strlen(buf), 0);
        if(ret < 0)
        {
            perror("send error!");
            return -1;
        }

        memset(buf, '\0', sizeof(buf));

        ret = recv(sockfd, buf, sizeof(buf)-1, 0);
        if(ret < 0)
        {
            perror("recv error!");
            return -1;
        }
        else if(ret == 0)
        {
            printf("peer shutdown!\n");
            close(sockfd);
            return 0;
        }

        printf("svr say# %s\n", buf);
    }

    return 0;
}
