#include <arpa/inet.h>
#include <stdio.h>
#include <netdb.h>
#include <memory.h>
#include <unistd.h>

void host_info(char **argv) {
    struct addrinfo *p, *list_p, hints;
    char *address = argv[1];
    int rc;
    printf("Input URL is %s\n", address);
    memset(&hints,0,sizeof(struct addrinfo)); //init hints
    //hints.ai_family = AF_INET; //just IPv4
    hints.ai_socktype = SOCK_STREAM; //just TCP
    rc = getaddrinfo(address,NULL,&hints,&list_p);
    if (rc != 0) printf("Error in call getaddrinfo %s", gai_strerror(rc));
    int MAX_LINE = 100;
    char buf[MAX_LINE];
    for (p = list_p; p; p = p->ai_next) {
        getnameinfo(p->ai_addr, p->ai_addrlen, buf, MAX_LINE, NULL, 0, 0);
        printf("%s\n", buf);
    }
    freeaddrinfo(list_p);
}

/**
 * 客户端打开某个地址，获取调用过 chapter11_socket、connect 的 client fd
 * @param hostname
 * @param port
 * @return client fd， -1 表示
 */
int open_client_fd(char *hostname, char *port) {
    struct addrinfo hint, *list_p, *p;
    int client_fd;
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_family = AF_INET;
    hint.ai_flags = AI_ADDRCONFIG | AI_NUMERICSERV;
    int res = getaddrinfo(hostname, port, &hint, &list_p);
    printf("getaddrinfo %d\n", res);
    for (p = list_p; p; p = p->ai_next) {
        if ((client_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;
        if (connect(client_fd, p->ai_addr, p->ai_addrlen) != -1)
            break;
        close(client_fd);
    }
    freeaddrinfo(list_p);
    if (!p) return -1;
    else return client_fd;
}

/**
 * 服务端打开一个监听描述符
 * @param port 监听的端口
 * @return -1 表示获取描述符失败，其余表示获取到的绑定到端口的监听描述符
 */
int open_listen_fd(char *port) {
    struct addrinfo hint, *list_p, *p;
    int listen_fd, optval = 1;
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_family = AF_INET;
    hint.ai_flags = AI_PASSIVE | AI_ADDRCONFIG | AI_NUMERICSERV;
    int rc = getaddrinfo(NULL, port, &hint, &list_p);
    printf("getaddrinfo %d\n", rc);
    for (p = list_p; p; p = p->ai_next) {
        if ((listen_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
        if (bind(listen_fd, p->ai_addr, p->ai_addrlen) == 0)
            break;
        close(listen_fd);
    }
    freeaddrinfo(list_p);
    if (!p) return -1;
    if (listen(listen_fd, 1024) < 0) {
        close(listen_fd);
        return -1;
    }
    else return listen_fd;
}