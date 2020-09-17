#pragma  once

#include <iostream>
#include <string>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

class context{
  public:
    int fd;
    char buffer[1024];
    Request req;
    Response rsp;

    contest(int _fd):fd(_fd){

    }
};

class Sock{
  public:
    static int Socket()
    {
      int sock = socket(AF_INET,SOCK_STREAM,0);
      if(sock < 0){
        cerr << "socket error!" << endl;
        exit(3);
      }
      int opt = 1;
      setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
      return sock;
    }
    static void Bind(int sock, int port)
    {
      struct sockaddr_in local;
      bzero(&local,sizeof(local));

      local.sin_family = AF_INET;
      local.sin_port = htons(port);
      local.sin_addr.s_addr = htonl(INADDR_ANY);

      if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0){
        cerr << "bind error!" << endl;
        exit(4);
      }
    }
    static void Listen(int sock)
    {
      const int backlog = 5;
      if(listen(sock,backlog) < 0){
        cerr << "listen error!" << endl;
        exit(5);
      }
    }
    static int Accept(int sock)
    {
      struct sockaddr_in peer;
      socklen_t len = sizeof(peer);
      int new_sock = accept(sock,(struct sockaddr*)&peer,&len);
      if(new_sock < 0){
        cerr << "accept error!" << endl;
        return -1;
      }
      return new_sock;
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
      struct epoll_event ev;
      //先处理读事件，读事件处理完毕(request全部读取完成)，再来处理写事件

      for(int i = 0; i < num; i++){

        int fd = revs[i].data.fd;

        if(revs[i].events & EPOLLIN){
          if(fd == listen_sock){
            //处理链接事件
            int sock = Sock::Accept(listen_sock);
            if(sock >= 0){
              cout << "有新链接到来..." << endl;
              ev.events = EPOLLIN;
              ev.data.ptr = new context(fd);
              epoll_ctl(epfd, EPOLL_CTL_ADD,sock,&ev);  //将新的事件添加到rb，user->os
            }
          }
          else{
            //处理常规读取数据事件
            char buffer[10240];
            ssize_t s = recv(fd,buffer,sizeof(buffer)-1,0);
            if(s > 0){
              buffer[s] = '\0';
              cout << "###################################################" << endl;
              cout << buffer;
              cout << "###################################################" << endl;
              cout << "将关心读事件，改写为写事件" << endl;
              //1. 分析数据or分析报文or构建响应
              //2. switch event
              ev.events = EPOLLOUT;
              ev.data.fd = fd;
              epoll_ctl(epfd, EPOLL_CTL_MOD,fd,&ev);
              continue;
            }
            else if(s == 0){
              cout << "client quit..." << endl;
            }
            else{
              cout << "recv error..." << endl;
            }
            close(fd);
            epoll_ctl(epfd,EPOLL_CTL_DEL,fd,nullptr);
          }
        }
        else if(revs[i].events & EPOLLOUT){
          //处理写事件
          string rsp_line = "HTTP/1.1 200 OK\r\n";
          string rsp_header = "Content-Type:text/html;\r\n";
          string rsp_blank = "\r\n";
          string rsp_text = "<html><h2>Hello Epoll!</h2></html>\r\n";

          send(fd, rsp_line.c_str(), rsp_line.size(),0);
          send(fd, rsp_header.c_str(), rsp_header.size(), 0);
          send(fd, rsp_blank.c_str(), rsp_blank.size(), 0);
          send(fd, rsp_text.c_str(), rsp_text.size(), 0);

          close(fd);
          epoll_ctl(epfd,EPOLL_CTL_DEL,fd,nullptr);
          cout << "返回响应结束..." << endl;
        }
        else{
          //异常事件
          cout << "异常" << endl;
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
        int timeout = -1;
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
