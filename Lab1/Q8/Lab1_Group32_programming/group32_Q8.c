#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>

int *array;
int s1;
int s2;

int sum (int start, int end) {
    int sum = 0;
    for (int i = start; i < end; i++)
    {
        sum = sum + array[i];
    }
    return sum;
}

int main (int argc, char *argv[]) {

    // start timer
    struct timeval start, end;
    gettimeofday(&start, NULL);

    int N = atoi(argv[1]);
    array = malloc(sizeof(int) * N);

    // seed random generator
    srand(time(NULL));
    for (int i = 0; i < N; i++)
    {
        array[i] = rand() % 2;
    }

    // create pipes
    int p1[2];
    int p2[2];
    pipe(p1);
    pipe(p2);

    int pid = fork();

    if (pid == 0) {
        close(p1[0]);
        int s1 = sum(0, N/2);
        write(p1[1], &s1, sizeof(s1));
        close(p1[1]);
    }

    if (pid > 0) {
        pid = fork();
        if (pid == 0)
        {
            close(p2[0]);
            s2 = sum(N/2, N);
            write(p2[1], &s2, sizeof(s2));
            close(p2[1]);
        } else {
            read(p1[0], &s1, sizeof(s1));
            read(p2[0], &s2, sizeof(s2));
            printf("Total sum %d\n", s1 + s2);

            gettimeofday(&end, NULL);
            double time_taken = 
                ((end.tv_sec - start.tv_sec) * 1e6 + (end.tv_usec - start.tv_usec)) * 1e-6;
            printf("Time %f\n", time_taken);
        }
    }

    return 0;
}