#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#define CNT 3

struct Params
{
  pthread_mutex_t mutex;
  pthread_cond_t condvar;
  int *result;
  int *resStr[CNT];
};

void *reader(void *p)
{
	printf("READER started\n");
	struct Params* params = (struct Params*) p;
	FILE * fo = fopen("numbers.txt", "rt");
	if(!fo)
        	printf("Error open numbers.txt\n");
    	else
    	{
		while (!feof(fo))
		{
			char curr;
			fscanf( fo, "%c", &curr );
			if (curr == '\n')
			{
				printf("!!!\n");
			}
		}
	}
	fclose(fo); 
}

void *worker(void *p)
{
	struct Params* params = (struct Params*) p;
	;
}

void *writer(void *p)
{
	printf("WRITER started\n");
	struct Params* params = (struct Params*) p;
	FILE * fo = fopen("result.txt", "wt");
        if(!fo)
                printf("Error open result.txt\n");
        else
        {
		for (int i = 0; i < CNT; i++)
		{
			*params->result = *params->result + *params->resStr[i];
			fwrite(&params->result, sizeof(int),1, fo );
		}
	}
}

int main()
{
	printf("Program started!\n");
	struct Params params;
	pthread_mutex_init(&params.mutex, NULL);
	pthread_cond_init(&params.condvar, NULL);
	params.result = 0;
	int tmp[CNT] = {0};
	params.resStr[CNT] = tmp;

	pthread_t reader1;
	pthread_create(&reader1, NULL, reader, &params);
	pthread_join(reader1, NULL);
	
	pthread_t workers[CNT];
	for (int i = 0; i < CNT; i++)
	{	
		printf("WORKER %i started!\n", i);
		pthread_create(&workers[i], NULL, worker, &params);
        	pthread_join(workers[i], NULL);
	}

	pthread_t writer1;
        pthread_create(&writer1, NULL, writer, &params);
        pthread_join(writer1, NULL);


	pthread_mutex_destroy(&params.mutex); 
  	pthread_cond_destroy(&params.condvar);  
	return 0;
}
