#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include "Util.hpp"

using namespace std;

int main()
{
  char *content_length = getenv("Content-Length");
  cout << "content_length" << endl;
  if(nullptr != content_length){
    std::string str(content_length);
    int cl = Util::StringToInt(str);
    char c;
    std::string args;
    for(auto i = 0; i < cl; ++i){
      read(0,&c,1);
      args.push_back(c);
    }
    cout << "<html><h3>";
    cout << args << " : " << cl << endl;
    cout << "</h3></html>" << endl;
  }else{
    cout << "get content length error!" << endl;
  }
  return 0;
}
