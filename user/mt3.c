#include "kernel/types.h"
#include "user/user.h"

#define TOTAL_MEMORY (128 * 1024 * 1024) // Physical Memeory 128MB
#define NULL 0

typedef void* (*malloc_func_t)(uint);
typedef void (*free_func_t)(void*);
typedef void (*get_free_func_t)();

void simulate_memory_operations();
void mutil(); // get memory utility

malloc_func_t u_malloc = wf_malloc;
free_func_t u_free = wf_free;
get_free_func_t u_get_free = wf_get_memory_fragments;

void _mt() {
	const int N = 1e4;
	uint t1 = uptime();
	char** ptr = u_malloc(N * sizeof(char*));
	for (int i = 0; i < N - 1; ++i) {
		ptr[i] = u_malloc((i + 1) * sizeof(char));
	}
	for (int i = N - 2; i >= 0; i -= 2) u_free(ptr[i]);
	for (int i = N - 3; i >= 0; i -= 2) u_free(ptr[i]);
	ptr[N - 1] = u_malloc(N * sizeof(char));
	uint t2 = uptime();
	printf("Malloc Test Cost %d Ticks\n", t2 - t1);

	simulate_memory_operations();
	u_get_free();
}

void simulate_memory_operations() {
  const int allocation_sizes[] = {10, 52, 100, 300, 700, 1025, 2049, 3022, 4097, 5120, 6144, 7168, 8192, 16384, 24333, 32768, 65536};
  const int num_allocations = sizeof(allocation_sizes) / sizeof(allocation_sizes[0]);
  const int num_initial_allocations = 80;
  const int num_fragmentation_rounds = 120;

  void *allocated_blocks[num_initial_allocations];

  // Step 1: Initial large allocations to set up the stage
  printf("simulate_memory_operations: Initial large allocations\n");
  for (int i = 0; i < num_initial_allocations; i++) {
    int size_index = i % num_allocations;
    allocated_blocks[i] = u_malloc(allocation_sizes[size_index] * sizeof(char));
    if (allocated_blocks[i] == NULL) {
      printf("Out of memory during initial allocation\n");
      return;
    }
    //printf("Allocated (#%d, %d)\n", i, allocation_sizes[size_index]);
  }

  // Step 2: Free half of the initially allocated blocks to create large free spaces
  printf("simulate_memory_operations: Freeing some initial blocks to create fragmentation\n");
  for (int i = 0; i < num_initial_allocations; i += 2) {
    u_free(allocated_blocks[i]);
    //printf("Freed (#%d)\n", i);
  }

  // Step 3: Further allocate and free to create more fine-grained fragmentation
  printf("simulate_memory_operations: Further allocation and free cycles\n");
  for (int i = 0; i < num_fragmentation_rounds; i++) {
    int size_index = i % num_allocations;
    char *p = u_malloc(allocation_sizes[size_index] * sizeof(char));
    if (p == NULL) {
      printf("Out of memory during fragmentation allocation\n");
      return;
    }
    //printf("Allocated (#%d, %d)\n", i, allocation_sizes[size_index]);

    if (i % 3 == 0) {
      u_free(p);
      //printf("Freed (#%d, %d)\n", i, allocation_sizes[size_index]);
    }
  }

  mutil();
  printf("simulate_memory_operations: Finished\n");
}

void mutil() {
    int free_memory = freemem();
    int used_memory = TOTAL_MEMORY - free_memory;

    printf("Total Memory: %d bytes\n", TOTAL_MEMORY);
    printf("Free Memory: %d bytes\n", free_memory);
    printf("Used Memory: %d bytes\n", used_memory);

    int memory_utilization = ((uint64)used_memory * 100) / TOTAL_MEMORY;
    printf("Memory Utilization: %d%%\n", memory_utilization);
}

int main(int argc, char *argv[]) {
	printf("\nOrigin-Fit\n\n");
	u_malloc = malloc;
	u_free = free;
	u_get_free = get_memory_fragments;
	_mt();
	exit(0);
}
