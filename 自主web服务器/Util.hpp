#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>

class Util{
  public:
    static void MakeKV(std::unordered_map<std::string,std::string> &header_kv, std::string &str){
      std::size_t pos = str.find(":");
      if(std::string::npos == pos){
        return;
      }

      std::string key = str.substr(0,pos);
      std::string value = str.substr(pos+2);
      header_kv.insert({key,value});
    }
    static int StringToInt(std::string &str){
      std::stringstream ss(str);
      int len = 0;
      ss >> len;
      return len;
    }
};
