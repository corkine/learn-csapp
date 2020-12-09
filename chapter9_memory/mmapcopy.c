#include <stdio.h> //print
#include <unistd.h> //write
#include <fcntl.h> //open
#include <sys/stat.h> //stat
#include <sys/mman.h> //mmap

int main(int argc, char *argv[]) {
    char *name = argv[1];
    int fd;
    struct stat stat;
    printf("Printing %s\n",name);
    fd = open(name, O_RDONLY, 0);
    fstat(fd, &stat);
    char *bufp = mmap(NULL, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    write(1, bufp, stat.st_size);
}