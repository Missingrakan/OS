#pragma once

#include <iostream>
#include <string>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <unordered_map>
#include <pthread.h>

class Dict{
  private:
    std::unordered_map<std::string,std::string> dict;
  public:
    Dict()
    {
      dict.insert({
          {"apple", "苹果"},
          {"banana", "香蕉"},
          {"old", "老的"}
          });
    }
    std::string Search(const std::string &k)
    {
      std::unordered_map<std::string,std::string>::const_iterator it = dict.find(k);
      if(it == dict.end()){
        return "查不到";
      }
      return it->second;
    }
    ~Dict(){}
};
class Server{
  private:
    std::string ip;
    short port;
    int listen_sock;
    static Dict d;
  public:
    Server(std::string _ip,short _port):ip(_ip),port(_port),listen_sock(-1)
    {}
    void InitServer()
    {
      listen_sock = socket(AF_INET,SOCK_STREAM,0);
      if(listen_sock < 0){
        std::cerr << "socket error..." << std::endl;
        exit(2);
      }
      struct sockaddr_in local;
      bzero(&local,sizeof(local));
      local.sin_family = AF_INET;
      local.sin_port = htons(port);
      local.sin_addr.s_addr = inet_addr(ip.c_str());
      if(bind(listen_sock,(struct sockaddr*)&local,sizeof(local)) < 0){
        std::cerr << "bind error..." << std::endl;
        exit(3);
      }

      if(listen(listen_sock,5) < 0){
        std::cerr << "listen error..." << std::endl;
        exit(4);
      }
    }
    static void *ServiceIO(void *args)
    {
      int *p = (int*)args;
      int fd = *p;
      delete p;

      char buf[1024];
      while(true){
        ssize_t s = read(fd,buf,sizeof(buf)-1);
        if(s > 0){
          buf[s] = 0;
          std::string q = buf;
          if(q == "q" || q == "quit"){
            std::cout << "client quit ..." << std::endl;
            break;
          }
          std::string value = d.Search(q);
          std::cout << "client# " << q << "->" << value << std::endl;
          write(fd,value.c_str(),value.size());
        }
        else if(s == 0){
          std::cout << "client quit ..." << std::endl;
          break;
        }
        else{
          std::cerr << "read error ..." << std::endl;
          break;
        }
      }
      close(fd);
      std::cout << "service done ..." << std::endl;
    }
    void Start()
    {
      signal(SIGCHLD,SIG_IGN);
      for(;;){
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        int fd = accept(listen_sock,(struct sockaddr*)&peer,&len);
        if(fd < 0){
          std::cerr << "accept error..." << std::endl;
          continue;
        }
        std::cout << "get a linking ... [" << inet_ntoa(peer.sin_addr) << ":" <<ntohs(peer.sin_port) << "]" << std::endl;

        pthread_t tid;
        int *p = new int(fd);
        pthread_create(&tid,nullptr,ServiceIO,(void*)p);
        //pid_t id = fork();
        //if(id == 0){
          ////child
          //close(listen_sock);
          //ServiceIO(fd);
          //exit(0);
        //}
        //close(fd);
      }
    }
    ~Server()
    {
      if(listen_sock >=0){
        close(listen_sock);
      }
    }
};
Dict Server::d;
