#pragma once

#include <iostream>
#include <string>

//x op y = 
typedef struct Request{
  int x;
  int y;
  char op;
}Request_t;

typedef struct Response{
  int status; // 0 -1 -2 -3
  int result;
}Response_t;
