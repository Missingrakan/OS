#include <unistd.h>
#include "ChatClient.hpp"

void menu()
{
    std::cout << "*****************************************" << std::endl;
    std::cout << "****** 1. register ****** 2. login ******" << std::endl;
    std::cout << "****** 3. loginout ****** 4. exit  ******" << std::endl;
    std::cout << "*****************************************" << std::endl;
}

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

    while(1)
    {
        menu();
        int select = -1;
        std::cout << "please enter your select# ";
        fflush(stdout);
        std::cin >> select;
        if(select == 1)
        {
            int ret = uc.registerToSvr(ip);
            if(ret < 0)
            {
                LOG(WARNING, "please retry register") << std::endl;
            }
            else if(ret == 0)
            {
                LOG(INFO, "register success! please login...") << std::endl;
            }
            uc.closeFd();
        }
        else if(select == 2)
        {
            int ret = uc.loginToSvr(ip);
            if(ret < 0)
            {
                LOG(ERROR, "please retry login") << std::endl;
            }
            else if(ret == 0)
            {
                LOG(INFO, "login success, please chatting...") << std::endl;
                while(1)
                {
                    std::string msg;
                    std::cout << "please enter your msg# ";
                    fflush(stdout);
                    std::cin >> msg;

                    uc.sendUdpMsg(msg, ip);
                    uc.recvUdpMsg();
                }
            }
        }
        else if(select == 3)
        {
            //登出
        }
        else if(select == 4)
        {
            LOG(INFO, "exit chat client") << std::endl;
            exit(0);
        }
    }

    while(1)
    {
        sleep(1);
    }

    return 0;
}