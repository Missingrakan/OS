#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include "../../include/rest_rpc.hpp"
#include "../protocol.hpp"

using namespace rest_rpc;
using namespace rest_rpc::rpc_service;
using namespace std;

class Client
{
    public:
        Client()=default;
        ~Client(){};

        void Init(string& ip, uint32_t port)
        {
            ip_ = ip;
            port_ = port;
            id_ = 0;
            room_id_ = 0;
            piece_ = 'w';
        }

        /*
         * rpc调用要告诉服务器，客户端想要的
         * 1.调用的函数名称
         * 2.函数的传参
         * 3.函数需要的返回值
         * */

        uint32_t Register()
        {
            string name;
            cout << "please enter your name# ";
            cin >> name;

            string password;
            cout << "please enter your password# ";
            cin >> password;

            string password1;
            cout << "please enter your password again# ";
            cin >> password1;

            if(password != password1)
            {
                cout << "The password don't match, please enter again!!!" << endl;
                //规定用户id从 1000 开始分配，小于1000的都是错误的
                return 1;
            }

            uint32_t id = 0;

            try
            {
                rpc_client client(ip_, port_);
                bool ret = client.connect();
                if(!ret)
                {
                    std::cout << "connect timeout" << std::endl;
                    return 2;
                }

                id = client.call<uint32_t>("Rpc_Register", name, password);
                std::cout << "Player id is " << id << std::endl;
            }
            catch(const std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }

            return id;
        }

        uint32_t Login()
        {
            uint32_t id;
            cout << "please enter your id# ";
            cin >> id;

            id_ = id;

            string passwd;
            cout << "please enter your password# ";
            cin >> passwd;

            try
            {
                rpc_client client(ip_, port_);
                bool ret = client.connect();
                if(!ret)
                {
                    std::cout << "connect timeout" << std::endl;
                    return 2;
                }

                /*
                 * 此处返回值也采用用户id1
                 *          返回用户id > 1000，认为登录成功
                 *          返回用户id < 1000，认为登录失败
                 * */
                id = client.call<uint32_t>("Rpc_Login", id, passwd);
            }
            catch(const std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }

            return id;
        }

        void Game()
        {
            while(1)
            {
                PlayerMenu();
                int select = 0;
                scanf("%d",&select);

                switch (select)
                {
                    case 1:
                        {
                            if(Match())
                            {
                                PlayGame();
                            }
                            else
                            {
                                cout << "match failed, please start matching again" << endl;
                            }
                        }
                        break;
                    case 2:
                        {
                            cout << "happy to meet you next time, quit..." << endl;
                            return;
                        }
                        break;
                    default:
                        cout << "input choices error, please try again!" << endl; 
                        break;
                }
            }
        }
    private:

        void PlayerMenu()
        {
            cout << "***************************" << endl;
            cout << "*** 1.匹配       2.退出 ***" << endl;
            cout << "***************************" << endl;
            cout << "please select# ";
        }

        bool PushPlayerToMatchPool()
        {
            bool res = false;

            try
            {
                rpc_client client(ip_, port_);
                bool ret = client.connect();
                if(!ret)
                {
                    std::cout << "connect timeout" << std::endl;
                    return false;
                }

                res = client.call<bool>("Rpc_Match", id_);
            }
            catch(const std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }

            return res;
        }

        int CheckReady()
        {
            int res = 0;

            try
            {
                rpc_client client(ip_, port_);
                bool ret = client.connect();
                if(!ret)
                {
                    std::cout << "connect timeout" << std::endl;
                    return 2;
                }

                res = client.call<int>("Rpc_CheckReady", id_);
            }
            catch(const std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }

            return res;
        }

        void PopPlayerfromMatchPool()
        {
            try
            {
                rpc_client client(ip_, port_);
                bool ret = client.connect();
                if(!ret)
                {
                    std::cout << "connect timeout" << std::endl;
                    return;
                }

                client.call("Rpc_PopPlayerfromMatchPool", id_);
            }
            catch(const std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }
        }

        bool Match()
        {
            PushPlayerToMatchPool();

            int time_sec = 20;
            while(1)
            {
                //进入匹配池后玩家有两种状态：
                //      1.PLAYING ==> 3：匹配上了，可以打游戏了
                //      2.ONLINE  ==> 1：落单了，不能打游戏

                int ret = CheckReady();
                if(ret == 3)
                {
                    cout << "matching success! please start game..." << endl;
                    return true;
                }
                else if(ret == 1)
                {
                    cout << "this time don't match success, please rematch..." << endl;
                }
                else
                {
                    //可能匹配池中只有一个人，但匹配线程一直等待
                    if(time_sec <= 0)
                    {
                        cout << "matching timeout! please rematch..." << endl;

                        //超时了，将该玩家从匹配池移除
                        PopPlayerfromMatchPool();
                        break;
                    }

                    printf("matching, wait %02d.................\r", time_sec--);
                    fflush(stdout);
                    sleep(1);
                }
            }

            return false;
        }

        uint32_t GetRoomID()
        {
            uint32_t res = 0;

            try
            {
                rpc_client client(ip_, port_);
                bool ret = client.connect();
                if(!ret)
                {
                    std::cout << "connect timeout" << std::endl;
                    return 0;
                }

                res = client.call<uint32_t>("Rpc_GetRoomID", id_);
            }
            catch(const std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }

            room_id_ = res;
            return res;
        }

        char GetPiece()
        {
            char res = ' ';

            try
            {
                rpc_client client(ip_, port_);
                bool ret = client.connect();
                if(!ret)
                {
                    std::cout << "connect timeout" << std::endl;
                    return ' ';
                }

                res = client.call<char>("Rpc_GetPiece", id_, room_id_);
            }
            catch(const std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }

            piece_ = res;
            return res;
        }

        string GetBoard()
        {
            string res = "";

            try
            {
                rpc_client client(ip_, port_);
                bool ret = client.connect();
                if(!ret)
                {
                    std::cout << "connect timeout" << std::endl;
                    return "";
                }

                res = client.call<string>("Rpc_GetBoard", room_id_);
            }
            catch(const std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }

            return res;
        }

        void ViewBoard(const string& board)
        {
            cout << "    ";
            for(int i = 1; i <= BOARD_SIZE; ++i)
            {
                cout << i << "   ";
            }
            cout << endl;

            for(int i = 0; i < BOARD_SIZE; ++i)
            {
                for(int z = 0; z < BOARD_SIZE+1; ++z)
                {
                    cout << "----";
                }
                cout << endl;

                cout << i+1 << " | ";
                for(int j = 0; j < BOARD_SIZE; ++j)
                {
                    cout << board[i*BOARD_SIZE + j] << " | ";
                }

                cout << endl;
            }

            for(int i = 0; i < BOARD_SIZE+1; ++i)
            {
                cout << "----";
            }
            cout << endl;
        }

        bool IsMyTurn()
        {
            bool res = false;

            try
            {
                rpc_client client(ip_, port_);
                bool ret = client.connect();
                if(!ret)
                {
                    std::cout << "connect timeout" << std::endl;
                    return false;
                }

                res = client.call<bool>("Rpc_IsMyTurn", id_, room_id_);
            }
            catch(const std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }

            return res;
        }

        char Step(const int x, const int y)
        {
            char res = 'C';

            try
            {
                rpc_client client(ip_, port_);
                bool ret = client.connect();
                if(!ret)
                {
                    std::cout << "connect timeout" << std::endl;
                    return 'T';
                }

                res = client.call<char>("Rpc_Step", id_, room_id_, x, y);
            }
            catch(const std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }

            return res;
        }

        bool PosIsRight(const string& board, const int x, const int y)
        {
            return board[(x-1)*BOARD_SIZE + y - 1] == ' ' ? true : false;
        }

        char GetResult()
        {
            char res = 'C';

            try
            {
                rpc_client client(ip_, port_);
                bool ret = client.connect();
                if(!ret)
                {
                    std::cout << "connect timeout" << std::endl;
                    return 'T';
                }

                res = client.call<char>("Rpc_GetResult", room_id_);
            }
            catch(const std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }

            return res;
        }

        /* PlayGame()流程：
         * 1.通过id获取房间号，后续通过房间号获取棋盘
         * 2.通过id+房间号，判断拿的什么棋子
         *
         * 3.对战：循环落子
         *      3.1 获取棋盘
         *      3.2 展示棋盘
         *      3.3 判断是否该自己落子，是否为自己的回合
         *              不是，继续从3开始
         *              是，获取落子位置
         *      3.4 判断用户落子位置是否正确
         *      3.5 判断输赢情况：X O T C
         *      3.6 游戏结束，展示最终棋盘
         * */

        void PlayGame()
        {
            uint32_t room_id = GetRoomID();
            if(room_id < 1024)
            {
                return;
            }

            char piece = GetPiece();
            cout << "please start game, you take " << piece << endl;

            char result = 'C';
            while(1)
            {
                string board = GetBoard();

                ViewBoard(board);

                result = GetResult();
                if(result != 'C')
                {
                    break;
                }

                if(!IsMyTurn())
                {
                    cout << "please waiting, the another player is thinking..." << endl;
                    sleep(1);
                    continue;
                }

                int x, y;
                cout << "please enter the piece position, such as x,y: ";
                scanf("%d, %d", &x, &y);

                if(x < 1 || x > BOARD_SIZE || y < 1 || y > BOARD_SIZE)
                {
                    cout << "input error! please enter again. the range of x,y is [1,5]..." << endl;
                    continue;
                }

                if(!PosIsRight(board, x, y))
                {
                    cout << "The input position " << x << "," << y << " already has pieces! please input again..." << endl;
                    continue;
                }

                result = Step(x, y);
                if(result != 'C')
                {
                    break;
                }
            }

            //cout << "result: " << result << " win!!! "<< "your_piece: " << piece << endl;
            
            if(result == piece)
            {
                cout << "Victory!!!! well done. Again?" << endl;
            }
            else if(result == 'T')
            {
                cout << "Now it's a tie, bewell-matched. Again?" << endl;
            }
            else
            {
                cout << "Defeat!!!! Keep on fighting. Again?" << endl;
            }
        }


    private:
        string ip_;
        uint32_t port_;
        uint32_t id_;
        uint32_t room_id_;
        char piece_;    //保存用户棋子
};
