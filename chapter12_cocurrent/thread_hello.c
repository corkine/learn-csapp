#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *thread(void *arg) {
    printf("HELLO WORLD\n");
    return NULL;
}

int main(int argc, char **argv) {
    pthread_t  tid;
    pthread_create(&tid, NULL, thread, NULL);
    pthread_join(tid, NULL);
    exit(0);
}