#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <unordered_map>
#include <vector>

#include "log.hpp"
#include "../protocol.hpp"

using namespace std;

class Room
{
    public:
        Room()
        {}
        Room(const uint32_t p1, const uint32_t p2, uint32_t room_id)
            : player_one_(p1), player_two_(p2), room_id_(room_id)
        {
            memset(board_, ' ', sizeof(board_));
            cur_player_ = p1;

            result_ = 'C';

            piece_[0] = BLACK;
            piece_[1] = WHITE;
        }

        ~Room()
        {}

        char GetPiece(const uint32_t id)
        {
            int pos = (id == player_one_ ? 0 : 1);
            return piece_[pos];
        }

        string GetBoard()
        {
            string board;
            for(int i = 0; i < BOARD_SIZE; ++i)
            {
                for(int j = 0; j < BOARD_SIZE; ++j)
                {
                    board.push_back(board_[i][j]);
                }
            }

            return board;
        }

        uint32_t GetCurrent()
        {
            return cur_player_;
        }

        char Step(const uint32_t id, const int x, const int y)
        {
            if(cur_player_ != id)
            {
                return 'C';
            }

            board_[x-1][y-1] = GetPiece(id);

            //改变cur为另一个要下棋的人
            ChangeCurPlayer();

            result_ = IsOver(x-1, y-1);
            return result_;
        }

        void ChangeCurPlayer()
        {
            cur_player_ = (cur_player_ == player_one_ ? player_two_ : player_one_);
        }

        char IsOver()
        {
            for(int i = 0; i < BOARD_SIZE; ++i)
            {
                if(board_[i][0] != ' ' && (board_[i][0] == board_[i][1] && board_[i][1] == board_[i][2]
                            && board_[i][2] == board_[i][3] && board_[i][3] == board_[i][4]))
                {
                    return board_[i][0];
                }
            }
            
            for(int j = 0; j < BOARD_SIZE; ++j)
            {
                if(board_[0][j] != ' ' && (board_[0][j] == board_[1][j] && board_[1][j] == board_[2][j]
                            && board_[2][j] == board_[3][j] && board_[3][j] == board_[4][j]))
                {
                    return board_[j][0];
                }
            }

            if(board_[0][0] != ' ' && (board_[0][0] == board_[1][1] && board_[1][1] == board_[2][2]
                        && board_[2][2] == board_[3][3] && board_[3][3] == board_[4][4]))
            {
                return board_[0][0];
            }

            if(board_[0][4] != ' ' && (board_[0][4] == board_[1][3] && board_[1][3] == board_[2][2]
                        && board_[2][2] == board_[3][1] && board_[3][1] == board_[4][0]))
            {
                return board_[0][4];
            }

            for(int i = 0; i < BOARD_SIZE; ++i)
            {
                for(int j = 0; j < BOARD_SIZE; ++j)
                {
                    if(board_[i][j] == ' ')
                        return 'C';
                }
            }

            return 'T';
        }

        char JudgeRowOrCol(const string& board)
        {
            char row_tmp[BOARD_SIZE] = {0};

            for(int i = 0; i < BOARD_SIZE; ++i)
            {
                memset(row_tmp, '\0', BOARD_SIZE);
                strncpy(row_tmp, board.c_str() + (i*BOARD_SIZE), BOARD_SIZE);

                char* ret = strstr(row_tmp, "XXXXX");
                if(ret != NULL)
                {
                    return 'X';
                }

                ret = strstr(row_tmp, "OOOOO");
                if(ret != NULL)
                {
                    return 'O';
                }
            }

            return 'C';
        }

        char IsOver(const int x, const int y)
        {
            //判断行
            string row_board = GetBoard();
            char ret = JudgeRowOrCol(row_board);

            if(ret == 'X' || ret == 'O')
                return ret;

            //判断列
            string col_board;
            col_board.clear();
            for(int i = 0; i < BOARD_SIZE; ++i)
            {
                for(int j = 0; j < BOARD_SIZE; ++j)
                {
                    col_board.push_back(board_[j][i]);
                }
            }

            ret = JudgeRowOrCol(col_board);
            if(ret == 'X' || ret == 'O')
                return ret;

            //对角线
            
            vector<char> vec;
            char base_char = board_[x][y];
            vec.clear(); 
            vec.push_back(base_char);
            int tmp_x = x;
            int tmp_y = y;

            while(tmp_x-1 >= 0 && tmp_y-1 >=0)
            {
                if(board_[tmp_x-1][tmp_y-1] == base_char)
                {
                    vec.push_back(board_[tmp_x-1][tmp_y-1]);
                    tmp_x -= 1;
                    tmp_y -= 1;
                }
                else
                {
                    break;
                }
            }

            tmp_x = x;
            tmp_y = y;

            while(tmp_x+1 < BOARD_SIZE && tmp_y+1 < BOARD_SIZE)
            {
                if(board_[tmp_x+1][tmp_y+1] == base_char)
                {
                    vec.push_back(board_[tmp_x+1][tmp_y+1]);
                    tmp_x += 1;
                    tmp_y += 1;
                }
                else
                {
                    break;
                }
            }

            if(vec.size() >= 5)
            {
                return base_char;
            }

            vec.clear();
            tmp_x = x;
            tmp_y = y;

            vec.push_back(base_char);

            while(tmp_x-1 >= 0 && tmp_y+1 < BOARD_SIZE)
            {
                if(board_[tmp_x-1][tmp_y+1] == base_char)
                {
                    vec.push_back(board_[tmp_x-1][tmp_y+1]);
                    tmp_x -= 1;
                    tmp_y += 1;
                }
                else
                {
                    break;
                }
            }

            tmp_x = x;
            tmp_y = y;

            while(tmp_x+1 < BOARD_SIZE && tmp_y-1 >= 0)
            {
                if(board_[tmp_x+1][tmp_y-1] == base_char)
                {
                    vec.push_back(board_[tmp_x+1][tmp_y-1]);
                    tmp_x += 1;
                    tmp_y -= 1;
                }
                else
                {
                    break;
                }
            }

            if(vec.size() >= 5)
                return base_char;

            //不是行、列、对角线，判断棋盘是否下满
            
            for(int i = 0; i < BOARD_SIZE; ++i)
            {
                for(int j = 0; j < BOARD_SIZE; ++j)
                {
                    if(board_[i][j] == ' ')
                        return 'C';
                }
            }

            return 'T';
        }

        char GetResult()
        {
            return result_;
        }

    private:
        uint32_t player_one_;
        uint32_t player_two_;
        uint32_t room_id_;

        char board_[BOARD_SIZE][BOARD_SIZE];

        //记录当前是哪个玩家在下棋
        uint32_t cur_player_;

        //游戏结果
        // 1.黑子胜：X
        // 2.白字胜：O
        // 3.平局：T
        // 4.继续落子：C
        char result_;

        // 记录约定得黑白子，方便对棋盘进行操作
        // p1 ---> piece[0] = 'X'
        // p2 ---> piece[1] = 'O'
        char piece_[2];
};

class RoomManager
{
    public:
        RoomManager()
        {
            map_room_.clear();
            prepare_room_id_ = PREPARE_ROOM_ID;
            pthread_mutex_init(&room_lock_, nullptr);
        }

        ~RoomManager()
        {
            map_room_.clear();
            pthread_mutex_destroy(&room_lock_);
        }

        uint32_t CreateRoom(const uint32_t p1, const uint32_t p2)
        {
            pthread_mutex_lock(&room_lock_);

            uint32_t room_id = prepare_room_id_++;
            Room r(p1, p2, room_id);
            map_room_.insert({room_id, r});

            pthread_mutex_unlock(&room_lock_);

            return room_id;
        }

       char GetPlayerPiece(const uint32_t id, const uint32_t room_id)
       {
           return map_room_[room_id].GetPiece(id);
       }

       string GetPlayerBoard(const uint32_t room_id)
       {
           return map_room_[room_id].GetBoard();
       }

       bool IsMyTurn(const uint32_t id, const uint32_t room_id)
       {
           return map_room_[room_id].GetCurrent() == id ? true : false;
       }

        char Step(const uint32_t id, const uint32_t room_id, const int x, const int y)
        {
            return map_room_[room_id].Step(id, x ,y);
        }

        char GetResult(const uint32_t room_id)
        {
            return map_room_[room_id].GetResult();
        }
    private:
        unordered_map<uint32_t, Room> map_room_;
        uint32_t prepare_room_id_;

        pthread_mutex_t room_lock_;
};
