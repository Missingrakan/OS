#include <unistd.h>
#include "ChatClient.hpp"

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cout << "./ChatClient [ip]" << std::endl;
        return -1;
    }

    std::string ip = argv[1];

    if(ip.size() == 0)
    {
        LOG(ERROR, "Illegal ip, please retry start ChatClient") << std::endl;
        return -1;
    }

    UdpClient uc;
    uc.registerToSvr(ip);

    while(1)
    {
        sleep(1);
    }

    return 0;
}
