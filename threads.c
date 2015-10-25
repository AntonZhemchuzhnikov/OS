#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "queue.h"

#define WORK_CNT 3

//QUEUE//////////////////////////////////////////////////////////

/*Переменная ошибок*/
int errorQueueArray;
 
/*Инициализация очереди*/
void initQueueArray(QueueArray *F) {
    F->ukBegin = 0;
    F->ukEnd = 0;
    F->len = 0;
    errorQueueArray = okQueueArray;
}
 
/*Включение в очередь*/
void putQueueArray(QueueArray *F, queueArrayBaseType E) {
    /*Если очередь переполнена*/
    if (isFullQueueArray(F)) {
        return;
    }
    /*Иначе*/
    F->buf[F->ukEnd] = E;                                 // Включение элемента
    F->ukEnd = (F->ukEnd + 1) % SIZE_QUEUE_ARRAY;         // Сдвиг указателя
    F->len++;                                                // Увеличение количества элементов очереди
 
}
 
/*Исключение из очереди*/
void getQueueArray(QueueArray *F, queueArrayBaseType *E) {
    /*Если очередь пуста*/
    if (isEmptyQueueArray(F)) {
        return;
    }
    /*Иначе*/
    *E = F->buf[F->ukBegin];                              // Запись элемента в переменную
    F->ukBegin = (F->ukBegin + 1) % SIZE_QUEUE_ARRAY;     // Сдвиг указателя
    F->len--;                                                // Уменьшение длины
}
 
/*Предикат: полна ли очередь*/
int isFullQueueArray(QueueArray *F) {
    if (F->len == SIZE_QUEUE_ARRAY) {
        errorQueueArray = fullQueueArray;
        return 1;
    }
    return 0;
}
 
/*Предикат: пуста ли очередь*/
int isEmptyQueueArray(QueueArray *F) {
    if (F->len == 0) {
        errorQueueArray = emptyQueueArray;
        return 1;
    }
    return 0;
}

/////////////////////////////////////////////////////////////////

struct Params
{
    pthread_mutex_t mutexGET;
    pthread_mutex_t mutexPUT;
    pthread_cond_t condvarGET;
    pthread_cond_t condvarPUT;
    pthread_mutex_t mutexWR;
    pthread_cond_t condvarWR;
    int total;
    QueueArray q;
    QueueArray resultStr;
    int strCnt;
    int sumCnt;
    int allIsRed;
    int workingStringExists;
};

void* reader(void *p)
{
	struct Params* params = (struct Params*) p;
	printf("(READER) Started\n");
	FILE * fo = fopen("numbers.txt", "rt");
	if(!fo)
        	printf("(READER) Error open numbers.txt\n");
    else
   	{
		int cnt = 0;
		while (1)
		{
		    sleep(1);
		    void* curr = (void*) calloc(25, sizeof(char));
		    //sleep(1);
			if (fgets(curr, 25, fo) != NULL) {
			    pthread_mutex_lock(&params->mutexPUT);
				putQueueArray(&params->q, curr);
				pthread_mutex_unlock(&params->mutexPUT);
				cnt++;
		        printf("(READER) String red - %s", (char*)curr);
                pthread_cond_signal(&params->condvarPUT);
			}
			else {
			    params->strCnt = cnt;
				break;
			}
		}
		params->allIsRed = 1;
		printf("(READER) FINISHED\n");
		pthread_mutex_lock(&params->mutexWR);
	        pthread_cond_signal(&params->condvarWR);
	        pthread_mutex_unlock(&params->mutexWR);
    }
	fclose(fo);
}

void* worker(void *p)
{
    pthread_t ThreadId;
    ThreadId = pthread_self();
    printf("WORKER (%i) started!\n", (int)ThreadId);
	struct Params* params = (struct Params*) p;
	while(1) {
	    pthread_mutex_lock(&params->mutexPUT);
        pthread_cond_wait(&params->condvarPUT, &params->mutexPUT); 
        pthread_mutex_unlock(&params->mutexPUT);
	    void* tmp;
	    char* str;
	    while (!isEmptyQueueArray(&params->q)) {
	        params->workingStringExists++;
	        pthread_mutex_lock(&params->mutexGET);
	        getQueueArray(&params->q, &tmp);
	        printf("(WORKER %i) Find string by signal: %s", (int)ThreadId, (char*)tmp);
	        pthread_mutex_unlock(&params->mutexGET);
	        str = (char*)tmp;
	        int* sum = (int*)calloc(1, sizeof(int));
	        *sum = 0;
	        pthread_t ThreadId;
            ThreadId = pthread_self(); 
	        printf("(WORKER %i) Working string: %s", (int)ThreadId, str);
	        int i = 0;
	        while (str[i] != '\n')
	        {
		        if (str[i] != ' ') {
			        *sum += str[i] - '0';
		        }
		        i++;
	        }
	        pthread_mutex_lock(&params->mutexPUT);
	        putQueueArray(&params->resultStr, (void*)sum);
	        params->sumCnt++;
	        pthread_mutex_unlock(&params->mutexPUT);
	        printf("(WORKER %i) Current str sum = %i\n", (int)ThreadId, *sum);
	        params->workingStringExists--;
	        pthread_mutex_lock(&params->mutexWR);
	        pthread_cond_signal(&params->condvarWR);
	        pthread_mutex_unlock(&params->mutexWR);
	    } 
	}
}

void* writer(void *p)
{
    struct Params* params = (struct Params*) p;
	printf("(WRITER) Started\n");
	while(1) {
	    struct Params* params = (struct Params*) p;
	    pthread_mutex_lock(&params->mutexWR);
	    pthread_cond_wait(&params->condvarWR,&params->mutexWR);
	    printf("(WRITER) Started summing\n");
	    while (!isEmptyQueueArray(&params->resultStr)) {
	        void* s = (void*)calloc(1, sizeof(int));
	        getQueueArray(&params->resultStr, &s);
	        params->total += *(int*)s;
	        printf("(WRITER) Current Sum - %i\n", params->total);
	    }
	    
	    if (params->allIsRed && isEmptyQueueArray(&params->resultStr) && isEmptyQueueArray(&params->q) 
	            && (params->workingStringExists == 0)) {
	        printf("(WRITER) All strings has been worked! Start writing into file...\n");
	        break;
	    }
	    pthread_mutex_unlock(&params->mutexWR);
	}
	printf("(WRITER) SumTOTAL= %i\n", params->total);
	FILE * fo = fopen("result.txt", "wt");
    if(!fo)
        printf("(WRITER) Error open result.txt\n");
    else
    fwrite(&params->total, sizeof(int),1, fo );
	fclose(fo);
    printf("(WRITER) FINISHED\n");
}

int main()
{
	printf("Program started!\n");
	struct Params params;
	pthread_mutex_init(&params.mutexGET, NULL);
	pthread_cond_init(&params.condvarGET, NULL);
	pthread_mutex_init(&params.mutexPUT, NULL);
	pthread_cond_init(&params.condvarPUT, NULL);
	pthread_mutex_init(&params.mutexWR, NULL);
	pthread_cond_init(&params.condvarWR, NULL);
	params.total = 0;
	initQueueArray(&params.q);
	initQueueArray(&params.resultStr);
	params.strCnt = 0;
	params.sumCnt = 0;
	params.allIsRed = 0;
	params.workingStringExists = 0;

	pthread_t reader1;
	pthread_t workers[WORK_CNT];
	pthread_t writer1;
	
	for (int i = 0; i < WORK_CNT; i++)
	{	
		pthread_create(&workers[i], NULL, worker, &params);
        //pthread_join(workers[i], NULL);
	}
	
	pthread_create(&reader1, NULL, reader, &params);
	
	pthread_create(&writer1, NULL, writer, &params);
	

    pthread_join(writer1, NULL);
    pthread_join(reader1, NULL);


	pthread_mutex_destroy(&params.mutexGET); 
	pthread_mutex_destroy(&params.mutexPUT);
  	pthread_cond_destroy(&params.condvarPUT);  
  	pthread_cond_destroy(&params.condvarGET);
	return 0;
}
