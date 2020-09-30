#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include "Log.hpp"
#include "Util.hpp"

#define WWWROOT "./wwwroot"
#define WELCOME_PAGE "index.html"
#define PAGE_404 "./wwwroot/404.html"

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
    int fd;   //打开的文件资源
    bool cgi;
    std::string suffix;
  public:
    HttpRequest():blank("\n"), path(WWWROOT), cgi(false), file_size(0), fd(-1){
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
    int Getfd(){
      return fd;
    }
    int GetFileSize()
    {
      return file_size;
    }
    std::string GetQueryString(){
      return query_string;
    }
    std::string GetRequestBody(){
      return request_body;
    }
    std::string GetPath(){
      return path;
    }
    void SetPath(std::string _path){
      path = _path;
      struct stat st;
      stat(path.c_str(), &st);
      file_size = st.st_size;
      cgi = false;
      suffix = ".html";
    }
    std::string &GetSuffix(){
      return suffix;
    }
    //GET /INDEX.HTML HTTP/1.0\n  
    void RequestLineParse(){
      std::stringstream ss(request_line);
      ss >> method >> url >> version;
      std::cout << "Method: " << method << std::endl;
      std::cout << "url: " << url << std::endl;
      std::cout << "version: " << version << std::endl;
      if(url == "/"){
        url += WELCOME_PAGE;
      }
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
    bool OpenResources(){
      int ret = true;
      fd = open(path.c_str(),O_RDONLY);
      if(fd < 0){
        LOG(Error,"open resources failed!");
        ret = false;
      }
      return ret;
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
      std::cout << "debug path: " << path << std::endl;
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
        std::size_t pos = path.rfind(".");
        if(std::string::npos == pos){
          suffix = ".html";
        }else{
          suffix = path.substr(pos);
        }
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
      std::cout << "#####################start#####################" << std::endl;
      std::cout << "debug: " << request_line;
      std::cout << "debug: " << request_header;
      std::cout << "debug: " << blank;
      std::cout << "debug: " << request_body;
      std::cout << "debug, method: " << method << std::endl;
      std::cout << "debug, url: " << url << std::endl;
      std::cout << "debug, version: " << version << std::endl;

      std::cout << "debug, path: " << path << std::endl;
      std::cout << "debug, query_string: " << query_string << std::endl;
      std::cout << "#####################end#####################" << std::endl;
    }

    ~HttpRequest()
    {
      if(fd >= 0){
        close(fd);
      }
    }
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
    std::string &GetResponseLine(){
      return response_line;
    }
    std::string &GetResponseHeader(){
      return response_header;
    }
    std::string &GetBlank(){
      return blank;
    }
    std::string &GetResponseText(){
      return response_text;
    }
    void SetResponseLine(std::string line){
      response_line = line;
    }
    void SetRequeseText(std::string text){
      response_text = text;
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
    bool RecvLine(std::string &line)
    {
      char c = 'X';
      bool result = true;
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
          result = false;
          break;
        }
      }
      return result;
    }
    bool RecvHttpRequestLine(std::string &request_line)
    {
      return RecvLine(request_line);
    }
    bool RecvHttpRequestHeader(std::string &request_header)
    {
      std::string line = "";
      bool result = true;
      do{
          line = "";
          if(RecvLine(line)){
            if(line != "\n"){
            request_header += line;
            }
          }else{
            result = false;
            break;
          }
      }while(line != "\n");

      return result;
    }
    //读取http请求的请求行，请求报头，包括空行
    int RecvHttpRequest(HttpRequest *rq)
    {
      std::string request_line;
      std::string request_header;
      if(RecvHttpRequestLine(request_line) && RecvHttpRequestHeader(request_header)){
        rq->SetRequestLine(request_line);
        rq->SetRequestHeader(request_header);
        return 200;
      }
      return 404;
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
          content_length--;
        }
        rq->SetRequestBody(body);
      }
      rq->SetUrlToPath();
      rq->SetCGI();
    }
    void SendResponse(HttpRequest *rq, HttpResponse *rsp){
      std::string line = rsp->GetResponseLine();
      line += rsp->GetResponseHeader();
      line += rsp->GetBlank();
      send(sock, line.c_str(), line.size(),0);
      if(rq->IsCgi()){
        std::string text = rsp->GetResponseText();
        send(sock,text.c_str(),text.size(),0);
      }else{
        sendfile(sock, rq->Getfd(), nullptr, rq->GetFileSize());
      }
    }
    ~Connect()
    {
      if(sock >= 0){
        close(sock);
      }
    }
};

class Entry{
  public:
    static void MakeResponse(HttpRequest*rq, HttpResponse *rsp, int code)
    {
      std::string line = Util::GetStatusLine(code);

      if(code == 404){
        rq->SetPath(PAGE_404);
      }
      rsp->SetResponseLine(line);
      line = "Content-Type: ";
      line += Util::SuffixToType(rq->GetSuffix());
      line += "\r\n";
      rsp->AddResponseHeader(line);               
      line = "Content-Length: ";
      
      if(rq->IsCgi()){
        //TODO
        std::string text = rsp->GetResponseText();
        line += Util::IntToString(text.size());
        line += "\r\n";
        rsp->AddResponseHeader(line);
      }
      else{
        line += Util::IntToString(rq->GetFileSize());
        line += "\r\n";
        rsp->AddResponseHeader(line);
        rq->OpenResources();
      }
    }
    static void ProcessNormal(Connect *conn, HttpRequest *rq, HttpResponse *rsp)
    {
      //没有query_string,不是POST,path
      //MakeResponse(rq,rsp);
      //conn->SendResponse(rq,rsp);
    }
    static int ProcessCGI(Connect *conn, HttpRequest *rq, HttpResponse *rsp){
      std::string content_length;
      int read_pipe[2];
      int write_pipe[2];
      pipe(read_pipe);
      pipe(write_pipe);

      std::string args;
      if(rq->IsGet()){
        args = rq->GetQueryString();
      }else{
        args = rq->GetRequestBody();   
      }


      pid_t id = fork();
      if(id < 0){
        LOG(Error,"fork error!");
        return 404;
      }else if(id == 0){
        //child
        close(read_pipe[1]);
        close(write_pipe[0]);

        dup2(read_pipe[0],0);
        dup2(write_pipe[1],1);

        content_length = "Content-Length=";
        content_length += Util::IntToString(args.size());

        putenv((char*)content_length.c_str());

        std::string path = rq->GetPath();
        //增加约定，利用重定向计数来完成文件描述符的约定
        //通过0文件描述符读取，往1文件描述符打印
        execl(path.c_str(),path.c_str(),nullptr);

        //rq->path 这个是我们要让子进程执行的程序
        //参数为rq->query_string(GET) or rq->body(POST)
        exit(1);
      }else{
        //father
        close(read_pipe[0]);
        close(write_pipe[1]);

        for(auto i = 0; i < args.size(); ++i){
          write(read_pipe[1], &args[i], 1);
        }

        char c;
        std::string body;
        while(read(write_pipe[0], &c, 1) > 0){
          body.push_back(c);
        }
        rsp->SetRequeseText(body);
        //将body设置进response_body
        pid_t ret = waitpid(id,nullptr,0);
        if(ret < 0){
          LOG(Warning, "waitpid child failed!");
          return 404;
        }
      }
      return 200;
    }
    static void *HanderRequest(void *arg){
      int sock = *(int*)arg;

      int code = 200;
      Connect *conn = new Connect(sock);
      HttpRequest *rq = new HttpRequest();
      HttpResponse *rsp = new HttpResponse();

      code = conn->RecvHttpRequest(rq);
      if(code == 200){
        rq->RequestLineParse();
        rq->RequestHeaderParse();
      }else{
        LOG(Warning,"recv http request error");
        goto end;
      }
      if(!rq->IsMethodok()){
        code = 404;
        LOG(Warning,"request Method is Not ok!");
        goto end;
      }
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
        code = 404;
        LOG(Warning,"Path is not legal!");
        goto end;
      }
      //request读完，url解析完毕，cgi setdone
      //no cgi:没有参数，更不是POST，http request -> path
      //cgi : 带参，server要处理参数
      
      if(rq->IsCgi()){
        //CGI
        LOG(Normal,"exec by cgi!");
        code = ProcessCGI(conn,rq,rsp);
      }else{
        //non cgi
        LOG(Normal,"exec by noncgi!");
      }
end:
      MakeResponse(rq,rsp,code);
      conn->SendResponse(rq,rsp);
      //rq->Show();

      //recv request
      //parse request
      //make response
      //send response
      
      delete conn;
      delete rq;
      delete rsp;
    }
};
