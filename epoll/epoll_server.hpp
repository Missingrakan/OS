#pragma  once

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

class sock{
  public:
    static int Socket()
    {

    }
    static void Bind()
    {

    }
    static void Listen()
    {

    }
    static int Accept()
    {

    }
};

class epoll_server{
  private:
    int listen_sock;
    int port;
    int epfd;
  public:
    epoll_server(int _port=8080):port(_port),listen_sock(-1),epfd(-1)
    {}
    void init_server()
    {
      epfd = epoll_create(128);
      if(epfd < 0){
        cerr << "epoll error!" << endl;
        exit(2);
      }
      listen_sock = Sock::Socket();
      Sock::Bind(listen_sock,port);
      Sock::Listen(listen_sock);
    }
    void HandlerEvents(struct epoll_event revs[], int num)
    {
      for(int i = 0; i < num; i++){
        if(revs[i].events & EPOLLIN){
          if(revs[i].data.fd == listen_sock){
            //处理链接事件
          }
          else{
            //处理常规读取数据事件
          }
        }
        else if(revs[i].events & EPOLLOUT){
          //处理写事件
        }
        else{
          //异常事件
        }
      }
    }
    void start()
    {
      struct epoll_event ev;
      ev.events = EPOLLIN;
      ev.data.fd = listen_sock;
      epoll_ctl(epfd,EPOLL_CTL_ADD,listen_sock,&ev);

      struct epoll_event revs[128];
      for(;;){
        int num = 0;
        int timeout = 1000;
        switch((num = epoll_wait(epfd,revs,128,timeout))){
          case -1:
            cerr << "epoll_wait error" << endl;
            break;
          case 0:
            cout << "time out..." << endl;
            break;
          default:
            HandlerEvents(revs,num);
            break;
        }
      }
    }
    ~epoll_server()
    {
      if(listen_sock >= 0){
        close(listen_sock);
      }
      if(epfd >= 0){
        close(epfd);
      }
    }
};
