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

////////////////////////////////////////// ОСВОБОЖДАТЬ ПАМЯТЬ!!!!!

struct Params
{
    pthread_mutex_t mutexGET;
    pthread_mutex_t mutexPUT;
    pthread_cond_t condvarGET;
    pthread_cond_t condvarPUT;
    int total;
    QueueArray q;
    QueueArray resultStr;
    int strCnt;
    int sumCnt;
};

void* reader(void *p)
{
	/*читаем построчно и забиваем в очередь*/
	struct Params* params = (struct Params*) p;
	pthread_mutex_lock(&params->mutexPUT);
	printf("READER started\n");
	FILE * fo = fopen("numbers.txt", "rt");
	if(!fo)
        	printf("Error open numbers.txt\n");
    else
   	{
		int cnt = 0;
		//pthread_mutex_lock(&params->mutex);
		while (1) //feof всегда false=(((
		{
		    void* curr = (void*) calloc(25, sizeof(char));
		    //sleep(1);
			if (fgets(curr, 25, fo) != NULL) {
				putQueueArray(&params->q, curr);
				cnt++;
		        printf("strokaRead- %s", (char*)curr);
			}
			else {
			    params->strCnt = cnt;
				break;
			}
		}
	}
    pthread_mutex_unlock(&params->mutexPUT);
    sleep(1);
	for (int i = 0; i < WORK_CNT; i++)
            pthread_cond_signal(&params->condvarPUT);
	//free(curr);
	fclose(fo);
}

void* worker(void *p)
{
	/*разбиваем строку и суммируем*/
	struct Params* params = (struct Params*) p;
	pthread_mutex_lock(&params->mutexPUT);
    pthread_cond_wait(&params->condvarPUT, &params->mutexPUT);
    pthread_mutex_unlock(&params->mutexPUT);
	void* tmp;
	char* str;
	
	while (!isEmptyQueueArray(&params->q)) {
	    pthread_mutex_lock(&params->mutexGET);
	    getQueueArray(&params->q, &tmp);
	    pthread_mutex_unlock(&params->mutexGET);
	    str = (char*)tmp;
	    int* sum = (int*)calloc(1, sizeof(int));
	    *sum = 0;
	    printf("strWorking- %s", str);
	
	    int i = 0;
	    while (str[i] != '\n')
	    {
		    //printf("0sum = %i\n", sum);

		    if (str[i] != ' ')
		    {
			    *sum += str[i] - '0';
			    //printf("CURR = %c\n", str[i]);
		    }
		    i++;
	    }
	    pthread_mutex_lock(&params->mutexPUT);
	    putQueueArray(&params->resultStr, (void*)sum);
	    params->sumCnt++;
	    pthread_mutex_unlock(&params->mutexPUT);
	    printf("0sum = %i\n", *sum);
	} 
	if (isEmptyQueueArray(&params->q)) {
	    pthread_mutex_lock(&params->mutexGET);
	    pthread_cond_signal(&params->condvarGET);
	    pthread_mutex_unlock(&params->mutexGET);
	}
}

void* writer(void *p)
{
	/*суммиуем суммы строк*/
	printf("WRITER started\n");
	struct Params* params = (struct Params*) p;
	pthread_mutex_lock(&params->mutexGET);
	pthread_cond_wait(&params->condvarGET,&params->mutexGET);
	sleep(2);
	if (params->strCnt == params->sumCnt) {
	    printf("WRITER started summing\n");
	    while (!isEmptyQueueArray(&params->resultStr)) {
	        void* s = (void*)calloc(1, sizeof(int));
	            getQueueArray(&params->resultStr, &s);
	            params->total += *(int*)s;   
	    }
        printf("writerSumTOTAL= %i\n", params->total);
	    FILE * fo = fopen("result.txt", "wt");
            if(!fo)
                printf("Error open result.txt\n");
            else
		        fwrite(&params->total, sizeof(int),1, fo );
		    fclose(fo);
		    printf("WRITER finnished\n");
	}
	pthread_mutex_unlock(&params->mutexGET);
}

int main()
{
	printf("Program started!\n");
	struct Params params;
	pthread_mutex_init(&params.mutexGET, NULL);
	pthread_cond_init(&params.condvarGET, NULL);
	pthread_mutex_init(&params.mutexPUT, NULL);
	pthread_cond_init(&params.condvarPUT, NULL);
	params.total = 0;
	initQueueArray(&params.q);
	initQueueArray(&params.resultStr);
	params.strCnt = 0;
	params.sumCnt = 0;

	pthread_t reader1;
	pthread_t workers[WORK_CNT];
	pthread_t writer1;
	
	pthread_create(&reader1, NULL, reader, &params);
	
	pthread_create(&writer1, NULL, writer, &params);
	
	for (int i = 0; i < WORK_CNT; i++)
	{	
		printf("WORKER %i started!\n", i);
		pthread_create(&workers[i], NULL, worker, &params);
        //pthread_join(workers[i], NULL);
	}

    pthread_join(writer1, NULL);
    pthread_join(reader1, NULL);


	pthread_mutex_destroy(&params.mutexGET); 
	pthread_mutex_destroy(&params.mutexPUT);
  	pthread_cond_destroy(&params.condvarPUT);  
  	pthread_cond_destroy(&params.condvarGET);
	return 0;
}
