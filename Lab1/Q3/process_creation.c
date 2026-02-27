#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>

#define N 3

int main () {

    for (int i=0; i<N; i++) {
        fork();
        fork();
    }
    
    pid_t pid = getpid();

    printf("pid: %u\n", pid);

    return 0;
}