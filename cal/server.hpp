#include "protocol.hpp"
#include "comm.hpp"

class server{
  private:
    int port;
    int listen_sock;
  public:
    server(int _port = 8080) : port(_port),listen_sock(-1)
    {}
    void initServer()
    {
      listen_sock = Sock::Socket();
      Sock::Bind(listen_sock,port);
      Sock::Listen(listen_sock,BACK_LOG);
    }
    void service(int sock)
    {
      while(1){
        Request_t rq = {0,0,0};
        Response_t rsp = {0,0};
        ssize_t s =read(sock, &rq, sizeof(rq));
        if(s > 0){
          switch(rq.op){
            case '+':
              rsp.result = rq.x + rq.y;
              break;
            case '-':
              rsp.result = rq.x - rq.y;
              break;
            case '*':
              rsp.result = rq.x * rq.y;
              break;
            case '/':
              {
                if(rq.y == 0){
                  rsp.status = -1;
                }
                else{
                  rsp.result = rq.x/rq.y;
                }
              }
              break;
            case '%':
              {
                if(rq.y == 0){
                  rsp.status = -2;
                }
                else{
                  rsp.result = rq.x % rq.y;
                }
              }
              break;
            default:
              rsp.status = -3;
              break;
          }
          write(sock,&rsp,sizeof(rsp));
        }
        else if(s == 0){
          std::cout << "client quit!" << std::endl;
          break;
        }
        else{
          break;
        }
      }
    }
    void start()
    {
      signal(SIGCHLD, SIG_IGN);
      for(;;){
        int sock = Sock::Accept(listen_sock);
        if(sock >= 0){
          pid_t id = fork();
          if(id == 0)
          {
            close(listen_sock);
            service(sock);
            exit(0);
          }
        }
        close(sock);
      }
    }
    ~server()
    {
      if(listen_sock >= 0){
        close(listen_sock);
      }
    }
};
