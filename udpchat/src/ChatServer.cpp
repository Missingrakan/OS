#include "ChatServer.hpp"

int main()
{
    ChatServer* cs = new ChatServer();
    if(!cs)
    {
        printf("Init ChatServer Failed!\n");
        return -1;
    }
    cs->initServer();
    cs->start();
    return 0;
}
