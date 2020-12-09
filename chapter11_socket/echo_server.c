#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "librio.h"
int MAX_LINE = 1000;
extern int open_listen_fd(char *port);

void echo(int conn_fd) {
    size_t n;
    char buf[MAX_LINE];
    rio_t rio;
    Rio_readinitb(&rio, conn_fd);
    while ((n = Rio_readlineb(&rio, buf, MAX_LINE)) != 0) { //从套接字读取
        printf("Server received %d bytes\n", (int)n); //打印读取状态
        Rio_writen(conn_fd, buf, n); //写回套接字
    }
}

int main(int argc, char **argv) {
    int listen_fd, conn_fd;
    socklen_t client_len; //chapter11_socket 地址长度
    struct sockaddr_storage client_addr; //用来存放 chapter11_socket 地址的空间
    char client_hostname[MAX_LINE], client_port[MAX_LINE];
    if (argc != 2) { //如果传入参数不对，给提示信息
        printf("Usage: Put listen port as argv1\n"); exit(0);
    }
    listen_fd = open_listen_fd(argv[1]); //获取 listen_fd
    printf("Get listen_fd %d\n", listen_fd);
    while (1) {
        printf("Waiting to accept\n");
        //确定 chapter11_socket 地址空间长度作为 chapter11_socket 地址长度
        client_len = sizeof(struct sockaddr_storage); 
        //阻塞接受客户端请求
        conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len); 
                        MAX_LINE, client_port, MAX_LINE, 0); 
        //从链接获取 ClientName 和 Port 并打印
        getnameinfo((struct sockaddr *)&client_addr, client_len, client_hostname,
        printf("Connect to (%s,%s)\n", client_hostname, client_port);
        echo(conn_fd); //执行服务器 echo 行为
        close(conn_fd); //关闭此到客户端的连接
    }
}