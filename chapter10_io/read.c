#include <stdio.h>

int main() {
    char c[100];
    FILE *f = fopen("/mnt/c/Users/Corkine/CLionProjects/learnC/src/snooze.c","r+");
    fread(&c,sizeof(c),1,f);
    printf("Res %s", c);
}