#include "apue.h"
#include <setjmp.h>

jmp_buf env1, env2;
void function1(void);
void function2(void);

int main(void) {
  
  if (setjmp(env1) == 1)
    printf("jump from function1 to main\n");
  else if (setjmp(env1) == 2)
    printf("jump from function2 to main\n");

  function1();
  function2();

  return 0;
}

void function1(void) {

  if (setjmp(env2) == 2)
    printf("jump from function2 to function1\n");
  printf("in function1\n");
  //longjmp(env1, 1);
  function2();
}

void function2(void) {
  printf("in function2\n");
  longjmp(env2, 2);
  //  longjmp(env1, 2);
}
