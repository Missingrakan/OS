#include "selectServer.hpp"

void Usage(string proc)
{
  cout << "Usage: " << proc << " port" << endl;
}

// ./selectServer port
int main(int argc, char *argv[])
{
  if(argc != 2){
    Usage(argv[0]);
    exit(1);
  }
  selectServer *ss = new selectServer(atoi(argv[1]));
  ss->initServer();
  ss->start();

  return 0;
}
