#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>

typedef struct {
    off_t  offset;
    size_t bytes;
} request_t;

typedef struct {
    int fd;
    unsigned char *buf;      
    request_t *reqs;         
    int start_idx;           // inclusive
    int end_idx;             // exclusive
    int is_write;            // 1=write, 0=read
    volatile int *err_cnt;
} thread_arg_t;

static void die_errno(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

static double elapsed_sec(const struct timespec *t0, const struct timespec *t1) {
    double s  = (double)(t1->tv_sec - t0->tv_sec);
    double ns = (double)(t1->tv_nsec - t0->tv_nsec) / 1e9;
    return s + ns;
}

static void shuffle_ints(int *a, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = a[i];
        a[i] = a[j];
        a[j] = tmp;
    }
}

static void *worker(void *vp) {
    thread_arg_t *arg = (thread_arg_t *)vp;

    //buffer for reads
    unsigned char *scratch = NULL;
    size_t scratch_cap = 0;

    for (int i = arg->start_idx; i < arg->end_idx; i++) {
        off_t off = arg->reqs[i].offset;
        size_t len = arg->reqs[i].bytes;

        if (arg->is_write) {
            //buffer region [off, off+len)
            size_t done = 0;
            while (done < len) {
                ssize_t rc = pwrite(arg->fd, arg->buf + off + done, len - done, off + (off_t)done);
                if (rc < 0) {
                    __sync_fetch_and_add(arg->err_cnt, 1);
                    pthread_exit(NULL);
                }
                done += (size_t)rc;
            }
        } else {
            // read into scratch
            if (len > scratch_cap) {
                unsigned char *tmp = realloc(scratch, len);
                if (!tmp) {
                    __sync_fetch_and_add(arg->err_cnt, 1);
                    free(scratch);
                    pthread_exit(NULL);
                }
                scratch = tmp;
                scratch_cap = len;
            }

            size_t done = 0;
            while (done < len) {
                ssize_t rc = pread(arg->fd, scratch + done, len - done, off + (off_t)done);
                if (rc < 0) {
                    __sync_fetch_and_add(arg->err_cnt, 1);
                    free(scratch);
                    pthread_exit(NULL);
                }
                if (rc == 0) { 
                    __sync_fetch_and_add(arg->err_cnt, 1);
                    free(scratch);
                    pthread_exit(NULL);
                }
                done += (size_t)rc;
            }
        }
    }

    free(scratch);
    pthread_exit(NULL);
}

static size_t sum_bytes(const request_t *reqs, int count) {
    size_t total = 0;
    for (int i = 0; i < count; i++) total += reqs[i].bytes;
    return total;
}

static void run_phase(
    const char *list_name,
    int is_write,
    const char *filename,
    int p,
    unsigned char *buf,
    request_t *reqs,
    int req_count
) {
    int fd = open(filename,
                  is_write ? (O_CREAT | O_TRUNC | O_RDWR) : O_RDONLY,
                  0644);
    if (fd < 0) die_errno("open");

    pthread_t *tids = calloc((size_t)p, sizeof(*tids));
    thread_arg_t *args = calloc((size_t)p, sizeof(*args));
    if (!tids || !args) die_errno("calloc");

    volatile int err_cnt = 0;

    // Split requests evenly among threads
    int per = (req_count + p - 1) / p;

    struct timespec t0, t1;
    if (clock_gettime(CLOCK_MONOTONIC, &t0) != 0) die_errno("clock_gettime");

    for (int tid = 0; tid < p; tid++) {
        int start = tid * per;
        int end = start + per;
        if (end > req_count) end = req_count;

        args[tid].fd = fd;
        args[tid].buf = buf;
        args[tid].reqs = reqs;
        args[tid].start_idx = start;
        args[tid].end_idx = end;
        args[tid].is_write = is_write;
        args[tid].err_cnt = &err_cnt;

        if (pthread_create(&tids[tid], NULL, worker, &args[tid]) != 0) {
            die_errno("pthread_create");
        }
    }

    for (int tid = 0; tid < p; tid++) {
        if (pthread_join(tids[tid], NULL) != 0) {
            die_errno("pthread_join");
        }
    }

    if (clock_gettime(CLOCK_MONOTONIC, &t1) != 0) die_errno("clock_gettime");
    close(fd);

    free(tids);
    free(args);

    size_t bytes = sum_bytes(reqs, req_count);
    double sec = elapsed_sec(&t0, &t1);
    double bw = (sec > 0.0) ? ((double)bytes / (1024.0 * 1024.0)) / sec : 0.0;

    if (is_write) {
        printf("%s: Write %zu bytes, use %d threads, elapsed time %.6f s, write bandwidth: %.3f MB/s",
               list_name, bytes, p, sec, bw);
    } else {
        printf("%s: Read  %zu bytes, use %d threads, elapsed time %.6f s, read bandwidth:  %.3f MB/s",
               list_name, bytes, p, sec, bw);
    }

    if (err_cnt) printf(" (errors=%d)", (int)err_cnt);
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <N_bytes> <P_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    long n_long = atol(argv[1]);
    int p = atoi(argv[2]);
    if (n_long <= 0 || p <= 0) {
        fprintf(stderr, "Error: N_bytes and P_threads must be > 0\n");
        return EXIT_FAILURE;
    }

    const size_t N = (size_t)n_long;
    const char *filename = "io_perf_test.bin";

    // Allocate and initialize buffer of N bytes
    unsigned char *buf = malloc(N);
    if (!buf) die_errno("malloc");
    for (size_t i = 0; i < N; i++) buf[i] = (unsigned char)(i & 0xFF);

    // Create two lists 
    enum { REQ_MAX = 100 };
    const size_t LIST1_SZ = 16384;
    const size_t LIST2_SZ = 128;
    const size_t BLOCK = 4096;

    //List1
    int list1_count = (int)(N / LIST1_SZ);
    if (list1_count > REQ_MAX) list1_count = REQ_MAX;
    if (list1_count < REQ_MAX) {
        fprintf(stderr, "Warning: N=%zu too small for 100x16384B; running List1 with %d requests.\n",
                N, list1_count);
    }

    request_t list1[REQ_MAX];
    for (int i = 0; i < list1_count; i++) {
        list1[i].offset = (off_t)i * (off_t)LIST1_SZ;
        list1[i].bytes  = LIST1_SZ;
    }

    //List2
    size_t blocks = N / BLOCK;
    int list2_count = (int)blocks;
    if (list2_count > REQ_MAX) list2_count = REQ_MAX;
    if (list2_count < REQ_MAX) {
        fprintf(stderr, "Warning: N=%zu provides only %zu blocks of 4096; running List2 with %d requests.\n",
                N, blocks, list2_count);
    }

    request_t list2[REQ_MAX];
    int *ids = malloc(blocks * sizeof(int));
    if (!ids) die_errno("malloc(ids)");
    for (size_t i = 0; i < blocks; i++) ids[i] = (int)i;

    srand((unsigned int)time(NULL));
    shuffle_ints(ids, (int)blocks);

    for (int i = 0; i < list2_count; i++) {
        list2[i].offset = (off_t)ids[i] * (off_t)BLOCK;
        list2[i].bytes  = LIST2_SZ;
    }
    free(ids);

    //write then read tests
    run_phase("List1", 1, filename, p, buf, list1, list1_count);
    run_phase("List1", 0, filename, p, buf, list1, list1_count);

    run_phase("List2", 1, filename, p, buf, list2, list2_count);
    run_phase("List2", 0, filename, p, buf, list2, list2_count);

    free(buf);
    return 0;
}