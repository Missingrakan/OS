#include <fstream>
#include <iostream>

#include "../../include/rest_rpc.hpp"
#include "hall.hpp"

using namespace rest_rpc;
using namespace rpc_service;
using namespace std;

Hall global_hall;

uint32_t Rpc_Register(rpc_conn conn, const string& name, const string& passwd)
{
    LOG(INFO, "") << name << ":" << passwd << endl;
    return global_hall.PlayerRegister(name, passwd);
}

uint32_t Rpc_Login(rpc_conn conn, const uint32_t id, const string& passwd)
{
    LOG(INFO, "recv login request... id is ") << id << " passwd is " << passwd << endl;
    return global_hall.PlayerLogin(id, passwd);

}

bool Rpc_Match(rpc_conn conn, uint32_t id)
{
    return global_hall.PushPlayerToMatchPool(id);
}

int Rpc_CheckReady(rpc_conn conn, uint32_t id)
{
    return global_hall.GetPlayerStatus(id);
}

void Rpc_PopPlayerfromMatchPool(rpc_conn conn, uint32_t id)
{
    return global_hall.PopPlayer(id);
}

uint32_t Rpc_GetRoomID(rpc_conn conn, uint32_t id)
{
    return global_hall.GetPlayerRoomID(id);
}

char Rpc_GetPiece(rpc_conn conn, uint32_t id, uint32_t room_id)
{
    return global_hall.GetPlayerPiece(id, room_id);
}

string Rpc_GetBoard(rpc_conn conn, uint32_t room_id)
{
    return global_hall.GetPlayerBoard(room_id);
}

bool Rpc_IsMyTurn(rpc_conn conn, uint32_t id, uint32_t room_id)
{
    return global_hall.IsMyTurn(id, room_id);
}

char Rpc_Step(rpc_conn conn, uint32_t id, uint32_t room_id, const int x, const int y)
{
    return global_hall.Step(id, room_id, x, y);
}

char Rpc_GetResult(rpc_conn conn, uint32_t room_id)
{
    return global_hall.GetResult(room_id);
}

int main()
{
    rpc_server server(9000, 2);

    server.register_handler("Rpc_Register", Rpc_Register);
    server.register_handler("Rpc_Login", Rpc_Login);
    server.register_handler("Rpc_Match", Rpc_Match);
    server.register_handler("Rpc_CheckReady", Rpc_CheckReady);
    server.register_handler("Rpc_PopPlayerfromMatchPool", Rpc_PopPlayerfromMatchPool);
    server.register_handler("Rpc_GetRoomID", Rpc_GetRoomID);
    server.register_handler("Rpc_GetPiece", Rpc_GetPiece);
    server.register_handler("Rpc_GetBoard", Rpc_GetBoard);
    server.register_handler("Rpc_IsMyTurn", Rpc_IsMyTurn);
    server.register_handler("Rpc_Step", Rpc_Step);
    server.register_handler("Rpc_GetResult", Rpc_GetResult);

    int ret = global_hall.InitHall();
    if(ret < 0)
    {
        LOG(ERROR, "The Game Hall start up failed! please check your logic!") << endl;
    }

    LOG(INFO, "Game Hall Start Success...") << "port:9000" <<endl;

    server.run();

    string str;
    cin >> str;
    return 0;
}
