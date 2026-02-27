#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/types.h>

#define N 1000000

int num_threads = 0;
float *array;

typedef struct pthread_sum
{
     float sum;
     int tid;
} pthread_sum;

float sum(int start, int end)
{
     float sum = 0.0;
     for (int i = start; i < end; i++)
     {
          sum = sum + array[i];
     }
     return sum;
}

void *thread_func(void *arg);

int main(int argc, char *argv[])
{
     num_threads = atoi(argv[1]);


     /* Initialize an array of random values */
     array = malloc(sizeof(float) * N);
     srand(time(NULL));
     for (int i = 0; i < N; i++)
     {
          float f = rand() % 2;
          array[i] = f;
     }

     /* Perform Serial Sum */
     float  sum_serial = 0.0;
     double time_serial = 0.0;

     //Timer Begin

     struct timeval start, end;
     gettimeofday(&start, NULL);

     sum_serial = sum(0, N);

     // Timer End
     gettimeofday(&end, NULL);
     time_serial =
         ((end.tv_sec - start.tv_sec) * 1e6 + (end.tv_usec - start.tv_usec)) * 1e-6;

     printf("Serial Sum = %f, time = %.3f \n", sum_serial, time_serial);

     /* Create a pool of num_threads workers and keep them in workers */ 
     pthread_t *workers;  
     pthread_sum *thread_data;
     workers = malloc(sizeof(pthread_t) * N);
     thread_data = malloc(sizeof(pthread_sum) * num_threads);
     double time_parallel = 0.0;
     double sum_parallel = 0.0;

     //Timer Begin
     gettimeofday(&start, NULL);

     for (int i = 0; i < num_threads; i++) {
          thread_data[i].tid = i;
          pthread_attr_t attr;
          pthread_attr_init(&attr);
          pthread_create(&workers[i], &attr, thread_func, &thread_data[i]);
     }

     
     for (int i = 0; i < num_threads; i++)  {
          pthread_join(workers[i], NULL);
          sum_parallel = sum_parallel + thread_data[i].sum;
     }


     //Timer End
     gettimeofday(&end, NULL);
     time_parallel =
         ((end.tv_sec - start.tv_sec) * 1e6 + (end.tv_usec - start.tv_usec)) * 1e-6;

     printf("Parallel Sum = %f, time = %.3f \n", sum_parallel, time_parallel);
     
     /*free up resources properly */

     free(array);
     free(workers);
     free(thread_data);
}

void *thread_func(void* arg) { 
     /* Assign each thread an id so that they are unique in range [0, num_thread -1 ] */

     pthread_sum *data = (pthread_sum *)arg;
     int my_id = data->tid;

     /* Perform Partial Parallel Sum Here */

     int range = N / num_threads;
     int start = my_id * range;
     int end = start + range;
     if (end > (N - range))
     {
          end = N;
     }
     
     float my_sum = sum(start, end);
     data->sum = my_sum;

     pthread_exit(NULL);
}