#include "epoll_server.hpp"

void Usage(string proc)
{
  cout << "Usage: " << proc << " port" << endl;
}
int main(int argc, char *argv[])
{
  if(argc != 2){
    Usage(argv[0]);
    exit(1);
  }
  epoll_server *ep_svr = new epoll_server(atoi(argv[1]));
  ep_svr->init_server();
  ep_svr->start();

  return 0;
}
