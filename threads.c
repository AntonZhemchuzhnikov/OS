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
    pthread_mutex_t mutex;
    pthread_cond_t condvar;
    int total;
    QueueArray q;
    QueueArray resultStr;
    int strCnt;
};

void *reader(void *p)
{
	/*читаем построчно и забиваем в очередь*/
	printf("READER started\n");
	struct Params* params = (struct Params*) p;
	FILE * fo = fopen("numbers.txt", "rt");
	void* curr;
	if(!fo)
        	printf("Error open numbers.txt\n");
    else
   	{
		curr = (void*) calloc(25, sizeof(char));
		int cnt = 0;
		pthread_mutex_lock(&params->mutex);
		while (1) //feof всегда false=(((
		{
			if (fgets(curr, 25, fo) != NULL)
			{
				putQueueArray(&params->q, curr);
				cnt++;
		        	//printf("strokaRead- %s\n", (char*)curr);
			}
			else
			{
			    params->strCnt = cnt;
				break;
			}
		}
		pthread_mutex_unlock(&params->mutex);
	}
	//free(curr);
	fclose(fo);
}

void *worker(void *p)
{
	/*разбиваем строку и суммируем*/
	struct Params* params = (struct Params*) p;
	void* tmp;
	char* str;
	getQueueArray(&params->q, &tmp);
	str = (char*)tmp;
	int* sum = (int*)calloc(3, sizeof(int));
	*sum = 0;
	printf("strWorking- %s\n", str);
	
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
	putQueueArray(&params->resultStr, (void*)sum); 
    printf("0sum = %i\n", *sum);
}

void *writer(void *p)
{
	/*суммиуем суммы строк*/
	printf("WRITER started\n");
	struct Params* params = (struct Params*) p;
	while (isEmptyQueueArray(&params->resultStr)) {
	    void* s = (void*)calloc(3, sizeof(int));
	    getQueueArray(&params->resultStr, &s);
	    params->total += *(int*)s;   
	    printf("writerSumStr= %i\n", params->total);
	}
	FILE * fo = fopen("result.txt", "wt");
        if(!fo)
            printf("Error open result.txt\n");
        else
		    fwrite(&params->total, sizeof(int),1, fo );
		fclose(fo);
}

int main()
{
	printf("Program started!\n");
	struct Params params;
	pthread_mutex_init(&params.mutex, NULL);
	pthread_cond_init(&params.condvar, NULL);
	params.total = 0;
	initQueueArray(&params.q);
	initQueueArray(&params.resultStr);
	params.strCnt = 0;

	pthread_t reader1;
	pthread_create(&reader1, NULL, reader, &params);
	pthread_join(reader1, NULL);
	
	pthread_t workers[WORK_CNT];
	for (int i = 0; i < WORK_CNT; i++)
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
