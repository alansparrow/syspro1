#include "apue.h"
#include <sys/wait.h>

int main(void) {
  pid_t pid;
  int status;
  int i;
  
  if ((pid = fork()) < 0)
    err_sys("fork error");
  else if (pid == 0)  /* child */
    exit(7);

  if ((i = wait(&status)) != pid)  /* wait for child */
    err_sys("wait error");
  printf("child process id: %d\n", pid);  /* child process */
  printf("parent(current) process id: %d\n", getpid());  /* parent(current) process */
  pr_exit(status);

  if ((pid = fork()) < 0)
    err_sys("fork error");
  else if (pid == 0)  /* child */
    abort();  /* generates SIGABRT */

  if (wait(&status) != pid)  /* wait for child */
    err_sys("wait error");
  pr_exit(status);  /* and print its status */

  if ((pid = fork()) < 0)
    err_sys("for error");
  else if (pid == 0)  /* child */
    status /= 0;  /* divide by 0 generates SIGFPE */

  if (wait(&status) != pid)  /* wait for child */
    err_sys("wait error");
  pr_exit(status);

  exit(0);
}
