#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/types.h>

/* Global variables */
int num_threads = 0;
int length = 0;
int num_bins = 30;
double bin_interval = 1.0 / 30.0;
double *array;

typedef struct pthread_histogram
{
     int bin[30];
     int tid;
} pthread_histogram;

int build_histogram(int start, int end, pthread_histogram *h)
{
     for (int i = start; i < end; i++)
     {
          int j = 0;
          double val = array[i];
          double b = bin_interval;
          while (val > b)
          {
               b = b + bin_interval;
               j++;
          }
          h->bin[j] = h->bin[j] + 1;
     }
     return 0;
}

void *thread_func(void *arg);

int main(int argc, char *argv[])
{
     num_threads = atoi(argv[1]);
     length = atoi(argv[2]);

     /* Initialize an array of random values */
     array = malloc(sizeof(double) * length);
     srand(time(NULL));
     for (int i = 0; i < length; i++)
     {
          array[i] = ((double)rand() / (double)RAND_MAX);
     }

     /* Serial histogram begin */
     double time_serial = 0.0;
     pthread_histogram serial_histogram = {{0}, 0};

     // Timer Begin
     struct timeval start, end;
     gettimeofday(&start, NULL);

     int histogram = build_histogram(0, length, &serial_histogram);

     // for (int k = 0; k < num_bins; k++)
     // {
     //      printf("%d. %d\n", k, serial_histogram.bin[k]);
     // }

     // Timer End
     gettimeofday(&end, NULL);
     time_serial =
         ((end.tv_sec - start.tv_sec) * 1e6 + (end.tv_usec - start.tv_usec)) * 1e-6;

     printf("Serial time = %.3f \n", time_serial);

     /* Serial histogram end*/

     /* Prallel hostogram start */

     /* Create a pool of num_threads workers and keep them in workers */
     pthread_t *workers;
     pthread_histogram *thread_data;
     workers = malloc(sizeof(pthread_t) * length);
     thread_data = malloc(sizeof(pthread_histogram) * num_threads);
     double time_parallel = 0.0;
     pthread_histogram parallel_histogram = {{0}, 0};

     // Timer Begin
     gettimeofday(&start, NULL);

     for (int i = 0; i < num_threads; i++)
     {
          thread_data[i].tid = i;
          pthread_attr_t attr;
          pthread_attr_init(&attr);
          pthread_create(&workers[i], &attr, thread_func, &thread_data[i]);
     }

     for (int i = 0; i < num_threads; i++)
     {
          pthread_join(workers[i], NULL);
          for (int j; j < num_bins; j++)
          {
               parallel_histogram.bin[j] = parallel_histogram.bin[j] + thread_data->bin[j];
          }
     }

     // //Timer End
     gettimeofday(&end, NULL);
     time_parallel =
         ((end.tv_sec - start.tv_sec) * 1e6 + (end.tv_usec - start.tv_usec)) * 1e-6;

     // for (int k = 0; k < num_bins; k++)
     // {
     //      printf("%d. %d\n", k, serial_histogram.bin[k]);
     // }
     printf("Parallel time = %.3f \n", time_parallel);

     /* free up resources */

     free(array);
     free(workers);
     free(thread_data);
}

void *thread_func(void *arg)
{
     /* Assign each thread an id so that they are unique in range [0, num_thread -1 ] */

     pthread_histogram *data = (pthread_histogram *)arg;
     int my_id = data->tid;

     /* Perform Partial Parallel Sum Here */

     int range = length / num_threads;
     int start = my_id * range;
     int end = start + range;
     if (end > (length - range))
     {
          end = length;
     }

     float my_his = build_histogram(start, end, data);
     // data->sum = my_sum;

     pthread_exit(NULL);
}