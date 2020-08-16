#include "udp_server.hpp"

void Usage(std::string arg)
{
  std::cout << arg << " ip port" << std::endl;
}

int main(int argc,char *argv[])
{
  if(argc != 3){
    Usage(argv[0]);
    exit(1);
  }
  Server *sv = new Server(argv[1],atoi(argv[2]));
  sv->InitServer();
  sv->Run();
  return 0;
}
