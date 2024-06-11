#include "kernel/types.h"
#include "user/user.h"

#define TOTAL_MEMORY (128 * 1024 * 1024) // Physical Memeory 128MB

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
  const int allocation_sizes[] = {600, 700, 700, 768, 1111, 1200, 1300, 1536, 2000, 2400, 3072, 4000};
  const int num_allocations = sizeof(allocation_sizes) / sizeof(allocation_sizes[0]);
  const int num_free_blocks = 80;
  const int N = 300;

  char** allocations = malloc(N * sizeof(char*));
  
  // Allocate memory
  printf("simulate_memory_operations: Allocate memory\n");
  for (int i = 0; i < num_free_blocks; i++) {
    int size_index = i % num_allocations;
    allocations[i] = malloc(allocation_sizes[size_index] * sizeof(char));
  }

  // Free some blocks to create fragment
  printf("simulate_memory_operations: Free some blocks to create fragment\n");
  for (int i = 0; i < num_free_blocks; i += 2) {
    free(allocations[i]);
  }

  // Further allocate and free to create fragmentation
  printf("simulate_memory_operations: Further allocate and free to create fragmentation\n");
  for (int i = num_free_blocks; i < num_free_blocks + 60; i++) {
    int size_index = i % num_allocations;
    allocations[i] = malloc(allocation_sizes[size_index] * sizeof(char));
    
    if (i % 3 == 0) {
      free(allocations[i]);
    }
  }

  printf("simulate_memory_operations: Finished");
  
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
	printf("\nWorse-Fit\n\n");
	_mt();
	printf("\nBest-Fit\n\n");
	u_malloc = bf_malloc;
	u_free = bf_free;
	u_get_free = bf_get_memory_fragments;
	_mt();
	exit(0);
}
