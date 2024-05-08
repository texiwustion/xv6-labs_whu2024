#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[]) {
  printf("The year when the Unix v6's release was published is %d\n", getyear());
  exit(0);
}
