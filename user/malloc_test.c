#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define ALLOC_SIZE 1024       // ?????????
#define NUM_ITERATIONS 10     // ???????
#define INNER_ITERATIONS 234  // ?????????????

void test_memory_allocation()
{
  void *ptrs[INNER_ITERATIONS];
  int i;

  for (i = 0; i < INNER_ITERATIONS; i++) {
    ptrs[i] = malloc(ALLOC_SIZE);
    if (ptrs[i] == 0) {
      printf("malloc failed at iteration %d\n", i);
      exit(0);
    }
  }

  for (i = 0; i < INNER_ITERATIONS; i++) {
    free(ptrs[i]);
  }

  exit(0);
}

int main(int argc, char* argv[])
{
  int i;
  int start, end, total_time;
  start = uptime();
  
  for (i = 0; i < NUM_ITERATIONS; i++) {
    if (fork() == 0) { // ???
      test_memory_allocation();
    } else {
      wait(0); // ???????
    }
  }
  
  end = uptime();
  total_time = end - start;
  printf("Total time: %d ticks\n", total_time);
  exit(0);
}
