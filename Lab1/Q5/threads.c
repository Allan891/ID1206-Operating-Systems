#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<stdio.h>

#define N 5

int value = 0;

void *thread_func(void *param);

int main(int argc, char *argv[]) {
    pid_t pid;
    pthread_t tid;
    pthread_attr_t attr;

    pid = fork();

    if (pid == 0) {
        pthread_attr_init(&attr);
        for(int i=0; i<N; i++) {
            pthread_create(&tid, &attr, thread_func, NULL);
            pthread_join(tid, NULL);
        }
        printf("CHILD: value = %d\n", value);
    } else if (pid > 0) {
        wait(NULL);
        printf("PARENT: value = %d\n", value);
    }
}

void *thread_func(void *param) {
    value++;
    pthread_exit(0);
}