#include "Client.hpp"
 
void Usage(std::string proc)
{
  std::cout << "Usage: " << proc << " server_ip server_port" << std::endl;
}
int main(int argc, char *argv[])
{
  if(argc != 3){
    Usage(argv[0]);
    exit(1);
  }

  int port = atoi(argv[2]);
  std::string ip = argv[1];

  Client *cp = new Client(ip,port);
  cp->InitClient();
  cp->Start();

  return 0;
}
