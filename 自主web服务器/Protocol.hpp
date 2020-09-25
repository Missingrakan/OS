#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include "Log.hpp"
#include "Util.hpp"

#define WWWROOT "./wwwroot"
#define WELCOME_PAGE "index.html"

class HttpRequest{
  private:
    std::string request_line;
    std::string request_header;
    std::string blank;
    std::string request_body;
  private:
    std::string method;
    std::string url;
    std::string version;

    std::unordered_map<std::string,std::string> header_kv;

    std::string path; //你这次请求想访问服务器上面的哪个资源
    std::string query_string; //你这次请求，想给服务器上面的的哪个资源传递参数
    int file_size;
    bool cgi;
  public:
    HttpRequest():blank("\n"), path(WWWROOT), cgi(false), file_size(0){
    }
    void SetRequestLine(std::string &line){
      request_line = line;
    }
    void SetRequestHeader(std::string &header){
      request_header = header;
    }
    void SetRequestBody(std::string &body){
      request_body = body;
    }
    void SetUrlToPath(){
      //if method == POST
      path += url;
    }
    void SetCGI(){
      cgi = true;
    }
    int GetFileSize()
    {
      return file_size;
    }
    //GET /INDEX.HTML HTTP/1.0\n  
    void RequestLineParse(){
      std::stringstream ss(request_line);
      ss >> method >> url >> version;
      std::cout << "Method: " << method << std::endl;
      std::cout << "url: " << url << std::endl;
      std::cout << "version" << version << std::endl;
    }
    void RequestHeaderParse(){
      size_t pos = request_header.find('\n');
      int start = 0;
      while(pos != std::string::npos){
        std::string sub = request_header.substr(start, pos-start);
        Util::MakeKV(header_kv,sub);
        start = pos + 1;
        pos = request_header.find('\n',pos+1);
      }
    }
    void UrlParse()
    {
      //url -> path, query_string(有可能不存在)
      std::size_t pos = url.find('?');
      if(std::string::npos == pos){
        path += url;  //不带参数
      }else{
        path += url.substr(0,pos); // /a/b?x=100
        query_string = url.substr(pos+1); //带参数的
        cgi = true;
      }
    }
    bool IsMethodok(){
      if(strcasecmp(method.c_str(),"GET") == 0 || strcasecmp(method.c_str(),"POST") == 0){
        return true;
      }
      return false;
    }
    bool IsCgi(){
      return cgi;
    }
    bool PathIsLegal()
    {
      bool ret = true;
      struct stat st;
      if(stat(path.c_str(),&st) == 0){
        //exist
        if(S_ISDIR(st.st_mode)){
          if(path[path.length()-1] != '/'){
            path += "/";
          }
          path += WELCOME_PAGE;
        }
        else if((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)){
          //可执行程序？POST，GET(带参的)
          cgi = true;
        }
        else{
          //OK
        }
        file_size = st.st_size;
      }else{
        //not exist 404
        ret = false;
      }
      return ret;
    }
    bool IsGet()
    {
      return strcasecmp(method.c_str(),"GET") == 0;
    }
    bool IsPost()
    {
      return strcasecmp(method.c_str(),"POST") == 0;
    }
    int GetContentLength()
    {
      //std::unordered_map<std::string,std::string> header_kv;
      auto it = header_kv.find("Content-Length");
      if(it == header_kv.end()){
        LOG(Warning,"Post method, but no request body!");
        return NORMAL_ERROR;
      }
      return Util::StringToInt(it->second);
    }
    void Show()
    {
      std::cout << "debug: " << request_line;
      std::cout << "debug: " << request_header;
      std::cout << "debug: " << blank;
      std::cout << "debug: " << request_body;
      std::cout << "debug, method: " << method << std::endl;
      std::cout << "debug, url: " << url << std::endl;
      std::cout << "debug, version: " << version << std::endl;

      std::cout << "debug, path: " << path << std::endl;
      std::cout << "debug, query_string: " << query_string << std::endl;
      }

    ~HttpRequest(){}
};

class HttpResponse{
  private:
    std::string response_line;
    std::string response_header;
    std::string blank;
    std::string response_text;
  public:
    HttpResponse():blank("\n"){
    }
    void SetResponseLine(std::string line){
      response_line = line;
    }
    void AddResponseHeader(std::string &line){
      if(response_header.empty()){
        response_header = line;
      }
      else{
        response_header += line;
      }
    }
    ~HttpResponse(){}
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
    int RecvLine(std::string &line)
    {
      char c = 'X';
      while(c != '\n'){
        ssize_t s = recv(sock,&c,1,0);
        if(s > 0){
          if(c == '\r'){
            recv(sock,&c,1,MSG_PEEK); //窥探
            if(c == '\n'){
              recv(sock,&c,1,0);
            }else{
              c = '\n';
            }
          }
          //normal char, \n, \r\n->\n, \r->\n
          line.push_back(c);
        }
        else{
          LOG(Warning,"recv request error!");
          break;
        }
      }
      return line.size();
    }
    void RecvHttpRequestLine(std::string &request_line)
    {
      RecvLine(request_line);
    }
    void RecvHttpRequestHeader(std::string &request_header)
    {
      std::string line = "";
      do{
          line = "";
          RecvLine(line);
          if(line != "\n"){
            request_header += line;
          }
      }while(line != "\n");
    }
    //读取http请求的请求行，请求报头，包括空行
    void RecvHttpRequest(HttpRequest *rq)
    {
      std::string request_line;
      std::string request_header;
      RecvHttpRequestLine(request_line);
      RecvHttpRequestHeader(request_header);

      rq->SetRequestLine(request_line);
      rq->SetRequestHeader(request_header);
    }
    void RecvHttpBody(HttpRequest *rq)
    {
      int content_length = rq->GetContentLength();
      if(content_length > 0){
        std::string body;
        char c;
        while(content_length > 0){
          recv(sock,&c,1,0);
          body.push_back(c);
        }
        rq->SetRequestBody(body);
      }
      rq->SetUrlToPath();
      rq->SetCGI();
    }
    ~Connect()
    {}
};

class Entry{
  public:
    static void MakeResponse(HttpRequest*rq, HttpResponse *rsp)
    {
      if(rq->IsCgi()){

      }
      else{
        std::string line = "HTTP/1.0 OK\r\n";
        rsp->SetResponseLine(line);
        line = "Content-Type: text/html\r\n";
        rsp->AddResponseHeader(line);
        line = "Content-Length: ";
        line += Util::IntToString(rq->GetFileSize());
        line += "\r\n";
        rsp->AddResponseHeader(line);
      }
    }
    static int ProcessNormal(Connect *conn, HttpRequest *rq, HttpResponse *rsp)
    {
      //没有query_string,不是POST,path
      MakeResponse(rq,rsp);

    }
    static void *HanderRequest(void *arg){
     int sock = *(int*)arg;
     Connect *conn = new Connect(sock);
     HttpRequest *rq = new HttpRequest();
     HttpResponse *rsp = new HttpResponse();

     conn->RecvHttpRequest(rq);

     if(!rq->IsMethodok()){
       LOG(Warning,"request Method is Not ok!");
     }
     //分析url:path, paramter
     rq->RequestHeaderParse();
     //url: 域名/资源文件?x=XX&&y=YY
     if(rq->IsPost()){
       //POST
       conn->RecvHttpBody(rq);
     }
     //request请求全部读完
     //1.分析请求资源是否合法
     if(rq->IsGet()){
       rq->UrlParse();
     }
     //2.分析请求路径中是否携带参数
     //rq->path
     if(!rq->PathIsLegal()){
       LOG(Warning,"Path is not legal!");
     }
     //request读完，url解析完毕，cgi setdone
     //no cgi:没有参数，更不是POST，http request -> path
     //cgi : 带参，server要处理参数
     
     if(rq->IsCgi()){
       //CGI
       LOG(Normal,"exec by cgi!");
     }
     else{
       //non cgi
       LOG(Normal,"exec by noncgi!");
       ProcessNormal(conn, rq, rsp);
     }

     rq->Show();

     //recv request
     //parse request
     //make response
     //send response     
     delete conn;
     delete rq;
     delete rsp;
    }
};
