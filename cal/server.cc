#include "server.hpp"

int main(int argc,char *argv[])
{
  if(argc != 2){
    std::cout << "Usage: " << argv[0] << " port" << std::endl;
    return 1;
  } 

  server *sp = new server(atoi(argv[1]));

  sp->initServer();
  sp->start();

  delete sp;

  return 0;
}
