#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    int MAX_LINE = 1000;
    char *buf, *p;
    char arg1[MAX_LINE], arg2[MAX_LINE], content[MAX_LINE];
    int n1 = 0, n2 = 0;
    if ((buf = getenv("QUERY_STRING")) != NULL) {
        p = strchr(buf, '&');
        *p = '\0';
        strcpy(arg1, buf);
        strcpy(arg2, p + 1);
        n1 = atoi(arg1);
        n2 = atoi(arg2);
    }
    sprintf(content, "QUERY_STRING=%s", buf);
    sprintf(content, "Welcome to add.com: ");
    sprintf(content, "%sThe Internet addition protal. \r\n<p>", content);
    sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", content, n1, n2, n1 + n2);
    sprintf(content, "%sThanks for visiting!\r\n", content);
    printf("Connection: close\r\n");
    printf("Content-length: %d\r\r",(int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s",content);
    fflush(stdout);
    exit(0);
}