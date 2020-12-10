#include "libserver.h"

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

int iter_server(int argc, char **argv) {
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
        //从链接获取 ClientName 和 Port 并打印
        getnameinfo((struct sockaddr *)&client_addr, client_len, client_hostname,
                        MAX_LINE, client_port, MAX_LINE, 0);
        printf("Connect to (%s,%s)\n", client_hostname, client_port);
        echo(conn_fd); //执行服务器 echo 行为
        close(conn_fd); //关闭此到客户端的连接
    }
}

void handle_signal(int sig_num) {
    while (waitpid(-1,0,WNOHANG) > 0)
        ;
}

int process_server(int argc, char **argv) {
    int listen_fd, conn_fd;
    socklen_t client_len;
    struct sockaddr_storage client_addr; //用来存放 chapter11_socket 地址的空间, Ipv4 or v6
    char client_hostname[MAX_LINE], client_port[MAX_LINE];
    if (argc != 2) { //如果传入参数不对，给提示信息
        printf("Usage: Put listen port as argv1\n"); exit(0);
    }
    signal(SIGCHLD, handle_signal);
    listen_fd = open_listen_fd(argv[1]); //获取 listen_fd
    printf("Get listen_fd %d\n", listen_fd);
    printf("Main Process Run in pid %d\n", getpid());
    while (1) {
        printf("Waiting to accept\n");
        client_len = sizeof(struct sockaddr_storage);
        conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
        getnameinfo((struct sockaddr *)&client_addr, client_len,
                client_hostname, MAX_LINE, client_port, MAX_LINE, 0);
        printf("Connect to (%s,%s)\n", client_hostname, client_port);
        if (fork() == 0) {
            close(listen_fd); //close listen_fd for subprocess
            printf("Conn_fd Run in pid %d\n", getpid());
            echo(conn_fd);
            close(conn_fd); //close conn_fd for subprocess, or os can auto close fds.
            exit(0); //MUST!!! or subprocess will route like main process!!!
        }
        close(conn_fd); //close conn_fd for main process
    }
}

void command(void) {
    char buf[MAX_LINE];
    if (!fgets(buf, MAX_LINE, stdin)) exit(0);
    printf("READ FROM STDIN: %s\n", buf);
}

int event_server(int argc, char **argv) {
    int listen_fd, conn_fd;
    socklen_t client_len;
    struct sockaddr_storage client_addr;
    char client_hostname[MAX_LINE], client_port[MAX_LINE];
    if (argc != 2) {
        printf("Usage: Put listen port as argv1\n"); exit(0);
    }
    listen_fd = open_listen_fd(argv[1]);
    printf("Get listen_fd %d\n", listen_fd);

    fd_set read_set, ready_set;
    FD_ZERO(&read_set);
    FD_SET(STDIN_FILENO, &read_set);
    FD_SET(listen_fd, &read_set);

    while (1) {
        printf("Waiting to accept\n");
        ready_set = read_set;
        select(listen_fd + 1, &ready_set, NULL, NULL, NULL);
        if (FD_ISSET(STDIN_FILENO, &ready_set)) command();
        if (FD_ISSET(listen_fd, &ready_set)) {
            client_len = sizeof(struct sockaddr_storage);
            conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
            getnameinfo((struct sockaddr *)&client_addr, client_len, client_hostname,
                        MAX_LINE, client_port, MAX_LINE, 0);
            printf("Connect to (%s,%s)\n", client_hostname, client_port);
            echo(conn_fd);
            close(conn_fd);
        }
    }
}

void *thread(void *vargp) {
    int conn_fd = *((int *)vargp);
    pthread_detach(pthread_self());
    free(vargp);
    echo(conn_fd);
    close(conn_fd);
    return NULL;
}

int thread_server(int argc, char **argv) {
    int listen_fd, *conn_dp;
    socklen_t client_len;
    struct sockaddr_storage client_addr;
    char client_hostname[MAX_LINE], client_port[MAX_LINE];
    pthread_t tid;
    if (argc != 2) {
        printf("Usage: Put listen port as argv1\n"); exit(0);
    }
    listen_fd = open_listen_fd(argv[1]);
    printf("Get listen_fd %d\n", listen_fd);
    while (1) {
        printf("Waiting to accept\n");
        client_len = sizeof(struct sockaddr_storage);
        conn_dp = malloc(sizeof(int));
        *conn_dp = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
        getnameinfo((struct sockaddr *)&client_addr, client_len, client_hostname,
                    MAX_LINE, client_port, MAX_LINE, 0);
        printf("Connect to (%s,%s)\n", client_hostname, client_port);
        pthread_create(&tid, NULL, thread, conn_dp);

    }
}

static int byte_cnt;
static sem_t mutex;

static void init_echo_cnt(void) {
    sem_init(&mutex, 0, 1);
    byte_cnt = 0;
}

void echo_cnt(int conn_fd) {
    int n;
    char buf[MAX_LINE];
    rio_t rio;
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    pthread_once(&once, init_echo_cnt);
    Rio_readinitb(&rio, conn_fd);
    while ((n = Rio_readlineb(&rio, buf, MAX_LINE)) != 0) {
        sem_wait(&mutex);
        byte_cnt += n;
        printf("Server received %d (%d total) bytes on fd %d\n", n, byte_cnt, conn_fd);
        sem_post(&mutex);
        Rio_writen(conn_fd, buf, n);
    }
}

sbuf_t sbuf;

void *thread_run(void *vargp) {
    pthread_detach(pthread_self());
    while (1) {
        int conn_fd = sbuf_remove(&sbuf);
        printf("thread %ld handle this conn now...\n", pthread_self());
        echo_cnt(conn_fd);
        close(conn_fd);
    }
}

int pre_thread_server(int argc, char **argv) {
    int i, listen_fd, conn_fd;
    socklen_t client_len;
    struct sockaddr_storage client_addr;
    char client_hostname[MAX_LINE], client_port[MAX_LINE];
    pthread_t tid;
    if (argc != 2) {
        printf("Usage: Put listen port as argv1\n"); exit(0);
    }
    listen_fd = open_listen_fd(argv[1]);
    printf("Get listen_fd %d\n", listen_fd);
    sbuf_init(&sbuf, S_BUF_SIZE);
    for (i = 0; i < N_THREADS; i++)
        pthread_create(&tid, NULL, thread_run, NULL);
    printf("Creating thread pool of %d\n", N_THREADS);
    while (1) {
        printf("Waiting to accept\n");
        client_len = sizeof(struct sockaddr_storage);
        conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
        getnameinfo((struct sockaddr *)&client_addr, client_len, client_hostname,
                    MAX_LINE, client_port, MAX_LINE, 0);
        printf("Connect to (%s,%s)\n", client_hostname, client_port);
        sbuf_insert(&sbuf, conn_fd);
    }
}

int main(int argc, char **argv) {
    pre_thread_server(argc, argv);
}