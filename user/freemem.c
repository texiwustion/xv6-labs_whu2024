#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("Free Memory: %d Bytes\n", freemem());

  exit(0);
}

