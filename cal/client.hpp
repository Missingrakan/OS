#include "protocol.hpp"
#include "comm.hpp"

class client{
  private:
    int sock;
    int svr_port;
    std::string svr_ip;
  public:
    client(std::string _ip = "127.0.0.1",int _port = 8080) : sock(-1),svr_ip(_ip),svr_port(_port)
    {}
    void initServer()
    {
      sock = Sock::Socket();
    }
    void start()
    {
      Sock::Connect(sock,svr_ip,svr_port);
      for(;;){
        Request_t rq;
        Response_t rsp;
        std::cout << "Enter your Data# " << std::endl;
        std::cin >> rq.x;
        std::cin >> rq.y;
        std::cout << "Enter your op# " << std::endl;
        std::cin >> rq.op;

        write(sock,&rq,sizeof(rq));
        ssize_t s = read(sock,&rsp,sizeof(rsp));
        if(s > 0){
          std::cout << "status# " << rsp.status << std::endl;
          std::cout << "result# " << rq.x << rq.op << rq.y << " = " << rsp.result << std::endl;
        }
        else{
          break;
        }
      }
      close(sock);
      sock = -1;
    }
    ~client()
    {
      if(sock >= 0){
        close(sock);
      }
    }
};
