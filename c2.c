#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>

#define WORKING_THREADS_COUNT 5

void *work(void *p) {
	int* param = (int*) p;
	*param = *param + 1;
	printf("WORKER %d has value %d\n", (int) pthread_self(), *param);
}

void main() {
	pthread_t workingThreads[WORKING_THREADS_COUNT];
	int param = 12;
	
	for (int i = 0; i < WORKING_THREADS_COUNT; i++)
	{
		pthread_create(&workingThreads[i], NULL, work, &param);
	}
	sleep(3);
}
