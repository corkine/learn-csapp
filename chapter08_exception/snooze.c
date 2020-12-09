#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

void handler(int signum) {
    return;
}

unsigned int snooze(unsigned int secs) {
    unsigned int res = sleep(secs);
    printf("Slept for %u of %d secs.",secs-res,secs);
    return res;
}

void main(int argc, char *argv[]) {
    signal(SIGINT,handler);
    signal(SIGTSTP,SIG_IGN);
    snooze(atoi(argv[1]));
    exit(0);
}