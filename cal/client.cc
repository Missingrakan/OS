#include "client.hpp"

void Usage(std::string proc)
{
  std::cout << "Usage: " << proc << " svr_ip svr_port" << std::endl;
}

//./client 127.0.0.1 8080 
int main(int argc, char *argv[])
{
  if(argc != 3){
    Usage(argv[0]);
    exit(1);
  }
  std::string ip = argv[1];
  int port = atoi(argv[2]);
  client *cp = new client(ip,port);
  cp->initServer();
  cp->start();
  return 0;
}
