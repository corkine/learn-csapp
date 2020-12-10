#include <stdlib.h>
#include <semaphore.h>
#include "librio.h"

typedef struct {
    int *buf; //Buffer Array
    int n; //Max Number of slots
    int front; //buf[(front+1)%n] first item
    int rear; //buf[rear%n] last item
    sem_t mutex; //Protects access to buf
    sem_t slots; //Counts available slots
    sem_t items; //Counts available items
} sbuf_t;

void sbuf_init(sbuf_t *sp, int n){
    sp->buf = calloc(n, sizeof(int));
    sp->n = n;
    sp->front = sp->rear = 0;
    sem_init(&sp->mutex, 0, 1);
    sem_init(&sp->items, 0, 0);
    sem_init(&sp->slots, 0, n);
}
void sbuf_de_init(sbuf_t *sp) {
    free(sp->buf);
}
void sbuf_insert(sbuf_t *sp, int item) {
    sem_wait(&sp->slots);
    sem_wait(&sp->mutex);
    sp->buf[(++sp->rear)%(sp->n)] = item;
    sem_post(&sp->mutex);
    sem_post(&sp->items);
}
int sbuf_remove(sbuf_t *sp) {
    int item;
    sem_wait(&sp->items);
    sem_wait(&sp->mutex);
    item = sp->buf[(++sp->front)%(sp->n)];
    sem_post(&sp->mutex);
    sem_post(&sp->slots);
    return item;
}

int reader_count = 0;
sem_t mutex, w;
void init(void) {
    sem_init(&mutex, 0, 1);
    sem_init(&w, 0, 1);
}

void reader(void) {
    while (1) {
        sem_wait(&mutex);
        reader_count++;
        if (reader_count == 1) sem_wait(&w);
        sem_post(&mutex);
        //do something read here...
        sem_wait(&mutex);
        reader_count--;
        if (reader_count == 0) sem_post(&w);
        sem_post(&mutex);
    }
}

void writer(void) {
    while (1) {
        sem_wait(&w);
        //do something write;
        sem_post(&w);
    }
}