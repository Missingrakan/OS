#include "client.hpp"

void Usage()
{
    cout << "./client [ip] [port]" << endl;
}

void Menu()
{
    cout << "*************************" << endl;
    cout << "*** 1.注册     2.登录 ***" << endl;
    cout << "*************************" << endl;
    cout << "*** 3.登出     4.退出 ***" << endl;
    cout << "*************************" << endl;
    cout << "please enter your choice # ";
}

int main(int argc, char* argv[]) 
{
    if(argc != 3)
    {
        Usage();
        return -1;
    }
    
    string ip = argv[1];
    int port = atoi(argv[2]);

    Client client;
    client.Init(ip, port);

    int select = 0;
    while(1)
    {
        Menu();
        scanf("%d", &select);
        switch (select)
        {
            case 1:
                //注册
                {
                    uint32_t id = client.Register();
                    if(id < 1000)
                    {
                        cout << "register failed, please retry!" << endl;
                    }
                    else
                    {
                        cout << "register success, please login..." << endl;
                    }
                }
                break;
            case 2:
                //登录
                {
                    uint32_t id = client.Login();
                    if(id < 1000)
                    {
                        cout << "Login failed, please check ID or password!" << endl;
                    }
                    else
                    {
                        //登录成功，可以开始下棋了
                        cout << "login success, please gameing..." << endl;
                        client.Game();
                    }
                }
                break;
            case 3:
                //登出
                break;
            case 4:
                //退出
                exit(1);
                break;
            default:
                cout << "input choices error, please try again" << endl;
                break;
        }
    }

    return 0;
}
