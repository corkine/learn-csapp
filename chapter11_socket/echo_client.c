#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "librio.h"
int MAX_LINE = 1000;
extern int open_client_fd(char *hostname, char *port);

int main(int argc, char **argv) {
    int client_fd;
    char *host, *port, buf[MAX_LINE];
    rio_t rio;
    if (argc != 3) {
        printf("Usage: Put host and port as argv1 and argv2\n"); exit(0);
    }
    host = argv[1];
    port = argv[2];
    client_fd = open_client_fd(host,port);
    Rio_readinitb(&rio, client_fd);
    while (fgets(buf,MAX_LINE,stdin) != NULL) { //从输入中读
        Rio_writen(client_fd,buf,strlen(buf)); //写到套接字
        Rio_readlineb(&rio,buf,MAX_LINE); //从套接字读
        fputs(buf, stdout); //将套接字返回输出
    }
    close(client_fd);
}