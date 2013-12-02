#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
  int newfd = dup(1);
  
  write(newfd, "hello world\n", 12);
  printf("newfd: %d\n", newfd);

  return 0;
}
