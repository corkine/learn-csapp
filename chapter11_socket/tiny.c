#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include "librio.h"

int MAX_LINE = 1000;
int MAX_BUF = 1000;
extern int open_listen_fd(char *port);

void client_error(int fd, char *filename, char *code, char *message1, char *message2) {
    char buf[MAX_LINE], body[MAX_BUF];
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, code, message1);
    sprintf(body, "%s<p>%s: %s\r\n", body, message2, filename);
    sprintf(body, "%s<hr><em>The Tiny Web Server</em>\r\n", body);

    sprintf(buf, "HTTP/1.0 %s %s\r\n", code, message1);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

void read_request_headers(rio_t *rp) {
    char buf[MAX_LINE];
    Rio_readlineb(rp, buf, MAX_LINE);
    while (strcmp(buf, "\r\n") != 0) {
        Rio_readlineb(rp, buf, MAX_LINE);
        printf("%s", buf);
    }
}

int parse_uri(char *uri, char *filename, char *cgi_args) {
    char *ptr;
    if (!strstr(uri, "cgi-bin")) { //静态内容
        strcpy(cgi_args, "");
        strcpy(filename, ".");
        strcat(filename, uri); //创建 Linux 相对路径名，比如 ./index.html
        if (uri[strlen(uri) - 1] == '/') //如果末尾为 /，则请求 **/home.html
            strcat(filename, "home.html");
        return 1;
    } else {
        ptr = index(uri, '?'); //找到查询字符串
        if (ptr) {
            strcpy(cgi_args, ptr + 1); //抽取查询字符串
            *ptr = '\0'; //拼接为字符串
        } else strcpy(cgi_args, ""); //简单设置为无查询字符串
        strcpy(filename, "."); //使用相对路径
        strcat(filename, uri); //拼接 Linux 相对路径
        return 0;
    }
}

void get_filetype(char *filename, char *filetype) {
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}

/**
 * Tiny 允许对 HTML 无格式文本文件 GIF PNG JPG 五种静态文件请求
 * @param fd 连接描述符
 * @param filename 文件名
 * @param filesize 文件大小
 */
void serve_static(int fd, char *filename, int filesize) {
    int src_fd;
    char *src_p, filetype[MAX_LINE], buf[MAX_BUF];
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));
    printf("Response headers: \n%s", buf);
    src_fd = open(filename, O_RDONLY, 0);
    src_p = mmap(0, filesize, PROT_READ, MAP_PRIVATE, src_fd, 0);
    close(src_fd);
    Rio_writen(fd, src_p, filesize);
    munmap(src_p, filesize);
}

void serve_dynamic(int fd, char *filename, char *cgi_args) {
    char buf[MAX_LINE], *empty_list[] = {NULL};
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
    if (fork() == 0) {
        setenv("QUERY_STRING", cgi_args, 1);
        dup2(fd, STDOUT_FILENO);
        execve(filename, empty_list, __environ);
    }
    wait(NULL);
}

void handle_request(int conn_fd) {
    int is_static;
    struct stat sbuf;
    char buf[MAX_LINE], method[MAX_LINE], uri[MAX_LINE], version[MAX_LINE];
    char filename[MAX_LINE], cgi_args[MAX_LINE];
    rio_t rio;

    Rio_readinitb(&rio, conn_fd);
    Rio_readlineb(&rio, buf, MAX_LINE);
    printf("Request headers:\n%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET") != 0) {
        client_error(conn_fd, method, "501", "Not Implemented", "Tiny not implemented this method");
        return;
    }
    read_request_headers(&rio);

    is_static = parse_uri(uri, filename, cgi_args);
    if (stat(filename, &sbuf) < 0) {
        client_error(conn_fd, filename, "404", "Not found.", "Tiny couldn't read this file.");
        return;
    }
    if (is_static) {
        if(!S_ISREG(sbuf.st_mode) || !(S_IRUSR & sbuf.st_mode)) {
            client_error(conn_fd, filename, "403", "Forbidden", "Tiny can't read this file.");
            return;
        }
        serve_static(conn_fd, filename, sbuf.st_size);
    } else {
        if (!S_ISREG(sbuf.st_mode) || !(S_IXUSR & sbuf.st_mode)) {
            client_error(conn_fd, filename, "403", "Forbidden", "Tiny can't run the CGI program.");
            return;
        }
        serve_dynamic(conn_fd, filename, cgi_args);
    }
}

int main(int argc, char **argv) {
    int listened_fd, conn_fd;
    char hostname[MAX_LINE], port[MAX_LINE];
    socklen_t client_len;
    struct sockaddr_storage client_addr;
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    listened_fd = open_listen_fd(argv[1]);
    while (1) {
        client_len = sizeof(client_addr);
        conn_fd = accept(listened_fd, (struct sockaddr *)&client_addr, &client_len);
        getnameinfo((struct sockaddr *)&client_addr, client_len, hostname, MAX_LINE, port, MAX_LINE, 0);
        printf("Accept connection from (%s, %s)\n", hostname, port);
        handle_request(conn_fd);
        close(conn_fd);
    }

}

