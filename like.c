#include <stdio.h>
#include <stdlib.h>
#ifdef __WINDOWS__
#include <conio.h>
#endif
int main()
{
  #ifdef __linux__
  printf("LINUX!!\n");
  #elif __WINDOWS__
  printf("WINDOWS!!\n");
  #endif
  return 0;
}
