#pragma once
#include <pthread.h>
#include <iostream>
#include <vector>

#include <ncurses.h>
#include "tools.hpp"
#include "ChatClient.hpp"

class ChatWindow;

class Pram
{
    public:
        Pram(int thread_num, ChatWindow* cw, UdpClient* uc)
        {
            thread_num_ = thread_num;
            cw_ = cw;
            uc_ = uc;
        }
    public:
        int thread_num_;
        ChatWindow* cw_;
        UdpClient* uc_;
};

class ChatWindow
{
    public:
        ChatWindow()
        {
            header_ = NULL;
            output_ = NULL;
            user_list_ = NULL;
            input_ = NULL;
            vec_.clear();

            pthread_mutex_init(&lock_win_, NULL);
        }

        ~ChatWindow()
        {
            if(header_)
            {
                delwin(header_);
            }

            if(output_)
            {
                delwin(output_);
            }

            if(user_list_)
            {
                delwin(user_list_);
            }

            if(input_)
            {
                delwin(input_);
            }

            pthread_mutex_destroy(&lock_win_);

            endwin();
        }

        //启动线程
        int start(UdpClient* uc)
        {
            //初始化ncurses
            initscr();
            for(int i = 0; i < 4; ++i)
            {
                Pram* pram = new Pram(i, this, uc);
                if(!pram)
                {
                    LOG(ERROR, "create parm failed") << std::endl;
                    exit(1);
                }

                pthread_t tid;
                int ret = pthread_create(&tid, NULL, runWindowStart, (void*)pram);
                if(ret < 0)
                {
                    LOG(ERROR, "create window thread failed") << std::endl;
                    exit(1);
                }

                vec_.push_back(tid);
            }

            for(size_t i = 0; i < vec_.size(); ++i)
            {
                //专门让调用start函数的客户端主线程在等待，否则主线程无事可干
                pthread_join(vec_[i], NULL);
            }

            return 0;
        }

        static void* runWindowStart(void* arg)
        {
            //需要区分不同的线程，让不同的线程去绘制不同的窗口
            
            Pram* pram = (Pram*)arg;
            int thread_num = pram->thread_num_;
            ChatWindow* cw = pram->cw_;
            UdpClient* uc = pram->uc_;

            switch(thread_num)
            {
                case 0:
                    cw->runHeader();
                    break;
                case 1:
                    cw->runOutput(uc);
                    break;
                case 2:
                    cw->runUserList(uc);
                    break;
                case 3:
                    cw->runInput(uc);
                    break;
            }

            delete pram;
            return NULL;
        }

        void runUserList(UdpClient* uc)
        {
            WINDOW* old_user_list = NULL;
            int line = 1;
            int cols = 1;
            while(1)
            {
                user_list_ = newwin((3*LINES)/5, COLS/4, LINES/5, (3*COLS)/4);
                box(user_list_, 0, 0);
                refreshWin(user_list_);

                if(old_user_list)
                {
                    delwin(old_user_list);
                }

                std::vector<UdpMsg> vec = uc->getVec();

                for(size_t i = 0; i < vec.size(); ++i)
                {
                    std::string msg;
                    msg += vec[i].nick_name_;
                    msg += ":";
                    msg += vec[i].school_;

                    mvwaddstr(user_list_, line+i, cols, msg.c_str());
                    refreshWin(user_list_);
                }

                old_user_list = user_list_;
                sleep(1);
            }
        }

        void runInput(UdpClient* uc)
        {
            WINDOW* old_input = NULL;
            while(1)
            {
                input_ = newwin(LINES/5, COLS, (LINES*4)/5, 0);
                box(input_, 0, 0);
                refreshWin(input_);

                if(old_input)
                {
                    delwin(old_input);
                }

                std::string tips = "please enter msg# ";
                mvwaddstr(input_, 1, 1, tips.c_str());
                refreshWin(input_);

                char buf[UDP_MAX_DATA_LEN] = {0};
                wgetnstr(input_, buf, sizeof(buf)-1);
                UdpMsg um;
                um.nick_name_ = uc->getMe().nick_name_;
                um.school_ = uc->getMe().school_;
                um.user_id_ = uc->getMe().user_id_;
                um.msg_.assign(buf, strlen(buf));

                std::string send_msg;
                um.serialize(&send_msg);

                uc->sendUdpMsg(send_msg);
                old_input = input_;

                sleep(1);
            }
        }

        void refreshWin(WINDOW* win)
        {
            pthread_mutex_lock(&lock_win_);
            wrefresh(win);
            pthread_mutex_unlock(&lock_win_);
        }

        void drowOutput()
        {
            output_ = newwin((LINES*3)/5, (3*COLS)/4, LINES/5, 0);
            box(output_, 0, 0);
            refreshWin(output_);
        }

        void runOutput(UdpClient* uc)
        {
            int line = 1;
            int cols = 1;

            drowOutput();
            int y, x;
            getmaxyx(output_, y, x);

            std::string msg;
            while(1)
            {
                msg.clear();

                //接收udp数据
                uc->recvUdpMsg(&msg);

                //udp消息的反序列化
                UdpMsg um;
                um.deserialize(msg);

                //设置展示格式：nick_name-school# msg
                std::string show_msg;
                show_msg += um.nick_name_;
                show_msg += "-";
                show_msg += um.school_;
                show_msg += "# ";
                show_msg += um.msg_;

                if(line >= y-2)
                {
                    drowOutput();
                    line = 1;
                }

                //把show_msg展示在output窗口
                mvwaddstr(output_, line, cols, show_msg.c_str());
                refreshWin(output_);

                line++;

                std::vector<UdpMsg> vec = uc->getVec();
                int flag = 1;
                //当服务端第一次给客户端推送消息时，此时客户端并没有保存在线用户列表，也就是说，vec.size() == 0
                
                for(size_t i = 0; i < vec.size(); ++i)
                {
                    //接收到的udp消息当中的用户id，和客户端保存的在线用户的用户id进行比较
                    if(um.user_id_ == vec[i].user_id_)
                    {
                        flag = 0;
                        break;
                    }

                    flag = 1;
                }

                if(flag == 1)
                {
                    uc->getVec().push_back(um);
                }
            }
        }

        void runHeader()
        {
            
            WINDOW* old_header = NULL;
            while(1)
            {
                header_ = newwin(LINES/5, COLS, 0, 0);

                if(old_header)
                {
                    delwin(old_header);
                }
                box(header_, 0, 0);
                refreshWin(header_);

                std::string msg = "welcome to chat system";
                int y, x;
                getmaxyx(header_, y, x);

                mvwaddstr(header_, y/2, (x-msg.size())/2, msg.c_str());
                refreshWin(header_);
                old_header = header_;

                sleep(1);
            }
        }

        
    private:
        WINDOW* header_;
        WINDOW* output_;
        WINDOW* user_list_;
        WINDOW* input_;

        pthread_mutex_t lock_win_;

        std::vector<pthread_t> vec_;
};
