#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<fcntl.h>

#define CONNMAX 10
#define BYTES 1024

char *ROOT;
int listenfd, clients[CONNMAX];
void error(char *);
void startServer(char *);
void respond(int);
int clntcnt = 0;

int main(int argc, char* argv[])
{
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    char c;    
    
    //по умолчанию PATH = ~/ , PORT=9999
    char PORT[6];
    ROOT = getenv("PWD");
    strcpy(PORT,"9999");

    int slot=0;

    //Парсинг аргументов командной строки
    while ((c = getopt(argc, argv, "p:r:")) != -1)
        switch (c) {
            case 'r': // корневая папка
                ROOT = malloc(strlen(optarg));
                strcpy(ROOT,optarg);
                break;
            case 'p': //порт
                strcpy(PORT,optarg);
                break;
            case '?': 
                printf(stderr,"Wrong arguments given!!!\n");
                exit(1);
            default:
                exit(1);
        }
    
    printf("Server started at port: %s with root directory: %s\n", PORT, ROOT);
    
    int i;
    for (i=0; i<CONNMAX; i++)
        clients[i]=-1;
    startServer(PORT);

    while (1) {
        addrlen = sizeof(clientaddr);
        clients[slot] = accept(listenfd, (struct sockaddr *) &clientaddr, &addrlen);

        if (clients[slot]<0)
            error ("accept() error");
        else {
            if (fork()==0) {
                respond(slot); 
                exit(0);
            }
        }
        while (clients[slot]!=-1) 
          slot = ++slot % CONNMAX;
    }
    return 0;
}

// функция запуска сервера
void startServer(char *port) {
    struct addrinfo hints, *res, *p;

    // получениие инф. хоста
    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo( NULL, port, &hints, &res) != 0) {
        perror ("getaddrinfo() error");
        exit(1);
    }
    // socket и bind
    for (p = res; p!=NULL; p=p->ai_next) {
        listenfd = socket(p->ai_family, p->ai_socktype, 0);
        if (listenfd != -1) 
          break;
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) 
          break;
    }
    if (p==NULL) {
        perror ("socket() or bind()");
        exit(1);
    }

    freeaddrinfo(res);

    // ожидание новых подключений
    if (listen(listenfd, 1000000) != 0) {
        perror("listen() error");
        exit(1);
    }
}

// подключение клиента
void respond(int n) {
    char mesg[999];
    char *reqline[3];
    char data_to_send[BYTES];
    char path[999];
    int rcvd, fd, bytes_read;

    memset((void*)mesg, (int)'\0', 999);

    rcvd=recv(clients[n], mesg, 999, 0);

    if (rcvd<0)    // получение ошибки
        fprintf(stderr,("recv() error\n"));
    else if (rcvd==0)    //  получение закрытого сокета
        fprintf(stderr,"Client disconnected upexpectedly.\n");
    else    // получение сообщения
    {
        clntcnt++;
        printf("Clients Count: %i\n", clntcnt);
        printf("Message: %s", mesg);
        reqline[0] = strtok(mesg, " \t\n");
        if (strncmp(reqline[0], "GET\0", 4)==0) {
            reqline[1] = strtok(NULL, " \t");
            reqline[2] = strtok(NULL, " \t\n");
            if (strncmp(reqline[2], "HTTP/1.0", 8)!=0 && strncmp(reqline[2], "HTTP/1.1", 8)!=0)
                write(clients[n], "400: Bad Request\n", 25);
            else {
                if (strncmp(reqline[1], "/\0", 2)==0)
                    reqline[1] = "/index.html";        //так как не указан файл, то будет открыт index.html

                strcpy(path, ROOT);
                strcpy(&path[strlen(ROOT)], reqline[1]);
                printf("file: %s\n", path);

                if ((fd=open(path, O_RDONLY))!=-1)    // файл найден
                {
                    send(clients[n], "200: OK\n\n", 17, 0);
                    while ((bytes_read=read(fd, data_to_send, BYTES))>0)
                        write (clients[n], data_to_send, bytes_read);
                }
                else    
                    write(clients[n], "404: Not Found\n", 23); // файл не найден
            }
        }
        printf("-------------------------------------\n");
    }

    //Закрытие сокета
    shutdown(clients[n], SHUT_RDWR);
    close(clients[n]);
    clients[n]=-1;
    clntcnt--;
}
