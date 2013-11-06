#include <unistd.h>
#include <stdio.h>
int main() {
  if (fork() == 0) {
    sleep(2);
    printf("child process!!! %d %d\n", getpid(), getppid());
  } else {
    while (1) {
      sleep(2);
      printf("my process num: %d, parent num: %d\n",
	     getpid(), getppid());
    }
  }
}
