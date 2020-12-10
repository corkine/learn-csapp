#include <arpa/inet.h>
#include <memory.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include "librio.h"

#define S_BUF_SIZE 16
#define MAX_LINE 1000
#define N_THREADS 4

typedef struct {
    int *buf; //Buffer Array
    int n; //Max Number of slots
    int front; //buf[(front+1)%n] first item
    int rear; //buf[rear%n] last item
    sem_t mutex; //Protects access to buf
    sem_t slots; //Counts available slots
    sem_t items; //Counts available items
} sbuf_t;

int open_client_fd(char *hostname, char *port);
int open_listen_fd(char *port);
void sbuf_init(sbuf_t *sp, int n);
void sbuf_de_init(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp);