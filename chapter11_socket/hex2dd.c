#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

char dst[20];

char *hex2dd(char *hex) {
    struct in_addr inaddr;
    uint32_t addr;
    //sscanf(hex, "%x", &addr);
    addr = strtoul(hex, NULL,16);
    addr = htonl(addr);
    inaddr.s_addr = addr;
    inet_ntop(AF_INET, &inaddr, dst, 20);
    return dst;
}

char *dd2hex(char *dd) {
    struct in_addr inaddr;
    inet_pton(AF_INET, dd, &inaddr);
    uint32_t res = ntohl(inaddr.s_addr);
    sprintf(dst,"0x%x",res);
    return dst;
}

int main(int argc, char **argv) {
    int opt;
    char *res;
    opt = (int)strtol(argv[1],NULL,10);
    if (opt == 1) res = hex2dd(argv[2]);
    else res = dd2hex(argv[2]);
    printf("%s\n", res);
}