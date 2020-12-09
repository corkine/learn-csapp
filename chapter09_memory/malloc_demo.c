#include <stdio.h>
#include <stdlib.h>

int main() {
int *array, i, n, sum = 0;
printf("Input number count >> ");
scanf("%d", &n);
printf("Input %d numbers\n",n);
array = (int *)malloc(n * sizeof(int));
for (i = 0; i < n; i++) {
    scanf("%d", &array[i]);
    sum += array[i];
}
free(array);
printf("Sum is %d\n", sum);
}