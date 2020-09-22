#pragma once

#include <iostream>

class HttpRequest{
  private:
    std::string request_line;
    std::string request_header;
    std::string blank;
    std::string request_text;
  
};

class HttpResponse{
  private:
    std::string status_line;
    std::string response_header;
    std::string blank;
    std::string response_text;
};

class Connect{
  private:
    int sock;
  public:
    Connect(int _sock):sock(_sock){}
    
    //1. \r
    //2. \r\n
    //3. \n
    //4. read one char
    int RecvLine(){

    }
    void RecvHttpRequest(HttpRequest *rq)
    {
    }
    ~Connect()
    {}
};

class Entry{
  public:
    static void *HanderRequest(void *arg){
     int sock = (int)arg;
     Connect *conn = new Connect(sock);
     HttpRequest *rq = new HttpRequest();
     HttpResponse *rsp = new HttpResponse();

     conn->RecvHttpRequest(rq);


     //recv request
     //parse request
     //make response
     //send response
     
     delete conn;
     delete rq;
     delete rsp;
    }
};
