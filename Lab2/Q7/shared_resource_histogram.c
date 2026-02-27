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
pthread_mutex_t lock;
int *serial_bins;
int *parallel_bins;

void build_histogram(int start, int end, int *histogram);
void print_histogram(int *h);
void *thread_func(void *arg);

int main(int argc, char *argv[])
{
    /* Initialize global variables */
    num_threads = atoi(argv[1]);
    length = atoi(argv[2]);
    pthread_mutex_init(&lock, NULL);
    serial_bins = malloc(sizeof(int) * num_bins);
    parallel_bins = malloc(sizeof(int) * num_bins);
    array = malloc(sizeof(double) * length);

    /* Initialize an array of random values */
    srand(time(NULL));
    for (int i = 0; i < length; i++)
    {
        array[i] = ((double)rand() / (double)RAND_MAX);
    }

    /* Serial histogram begin */
    /* Timer Begin */
    double time_serial = 0.0;
    struct timeval start, end;
    gettimeofday(&start, NULL);

    /* Create histogram */
    build_histogram(0, length, serial_bins);

    /* Timer End */
    gettimeofday(&end, NULL);
    time_serial =
        ((end.tv_sec - start.tv_sec) * 1e6 + (end.tv_usec - start.tv_usec)) * 1e-6;

    /* Print Serial histogram and time*/
    // print_histogram(serial_bins);
    printf("Serial time = %.3f \n", time_serial);
    /* Serial histogram end*/

    /* Prallel hostogram start */
    /* Create a pool of num_threads workers and keep them in workers */
    pthread_t *workers;
    workers = malloc(sizeof(pthread_t) * length);
    int *thread_id = malloc(sizeof(int) * num_threads);

    /* Timer Begin */
    double time_parallel = 0.0;
    gettimeofday(&start, NULL);

    /* Create threads */
    for (int i = 0; i < num_threads; i++)
    {
        thread_id[i] = i;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&workers[i], &attr, thread_func, &thread_id[i]);
    }

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(workers[i], NULL);
    }

    /* Timer End */
    gettimeofday(&end, NULL);
    time_parallel =
        ((end.tv_sec - start.tv_sec) * 1e6 + (end.tv_usec - start.tv_usec)) * 1e-6;

    /* Print parallel histogram and time */
    // print_histogram(parallel_bins);
    printf("Parallel time = %.3f \n", time_parallel);

    /* Print histogram data */
    // for (int i = 0; i < num_bins; i++)
    // {
    //     printf("%d. %d\n", i, parallel_bins[i]);
    // }

    /* Free up resources */
    free(array);
    free(workers);
    free(thread_id);
    free(parallel_bins);
    free(serial_bins);
    pthread_mutex_destroy(&lock);
}

void *thread_func(void *arg)
{
    /* Assign each thread an id so that they are unique in range [0, num_thread -1 ] */
    int my_id = *((int *)arg);

    /* Determin which chunk of the array to add */
    int range = length / num_threads;
    int start = my_id * range;
    int end = start + range;
    if (end > (length - range))
    {
        end = length;
    }

    /* Create histogram and then exit */
    build_histogram(start, end, parallel_bins);
    pthread_exit(NULL);
}

void build_histogram(int start, int end, int *histogram)
{
    /* Create temp local array */
    int local[30] = {0};
    for (int i = start; i < end; i++)
    {
        double val = array[i];
        double b = bin_interval;
        int j = 0;
        /* Step through bins in histogram until it matches the value. Increase j to determine index of the bin in histogram */
        while (val > b)
        {
            b = b + bin_interval;
            j++;
        }
        local[j] = local[j] + 1;
    }

    /* Critical section, use mutex lock while adding local histogram to the global one */
    pthread_mutex_lock(&lock);
    for (int i = 0; i < 30; i++)
    {
        histogram[i] = local[i] + histogram[i];
    }
    pthread_mutex_unlock(&lock);
}

void print_histogram(int *h)
{
    for (int i = 0; i < num_bins; i++)
    {
        if (i < 10)
        {
            printf(" %d. ", i);
        }
        else
        {
            printf("%d. ", i);
        }
        int val = h[i];
        while (val > 0)
        {
            printf("*");
            val--;
        }
        printf("\n");
    }
}