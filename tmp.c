#include "apue.h"
#include <stdio.h>

int main(void) {
#ifdef _SC_ATEXIT_MAX
  printf("%ld\n", sysconf(_SC_ATEXIT_MAX));
#else
  printf("no symbol for _SC_ATEXIT_MAX\n");
#endif
}
