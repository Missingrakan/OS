#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int i = 0;
    char bar[102];
    memset(bar,'\0',sizeof(bar));
    const char*lable = "|/-\\";
    int j=0;
    int color[]={1,2,3,4,5,6,7};
    for(; i <= 100; i++)
    {
      bar[i] = '*';
      printf("\033[3%dm[%-101s]\033[0m\033[33m[%d%%]\033[0m[%c]\r", color[j],bar,i,lable[i%4]);
      fflush(stdout);
      if(i%15 == 0){
        ++j;
      }
      usleep(100000);
    }
    printf("\n");
    return 0;
}
