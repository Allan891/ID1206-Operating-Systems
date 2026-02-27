#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>

int main(int argc, char** argv) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <num_pages>\n", argv[0]);
        exit(1);
    }

    struct timespec start, end;

    int num_pages = atoi(argv[1]);
    int page_size = getpagesize();

    printf("Allocating %d pages of %d bytes (normal pages)\n",
           num_pages, page_size);

    char *addr;

    // @Add the start of Timer here
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Option 2: @Add mmap below to allocate num_pages anonymous pages but using Ausing "huge" pages
    addr = (char*) mmap(NULL, num_pages * page_size,
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                        -1, 0);

    if (addr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    //the code below updates the pages
    char c = 'a';
    for (int i = 0; i < num_pages; i++) {
        addr[i * page_size] = c;
        c++;
        if (c > 'z') c = 'a';
    }

    // @Add the end of Timer here
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // holding nanosecond
    double elapsed = (end.tv_sec - start.tv_sec) +
                 (end.tv_nsec - start.tv_nsec) / 1e9;
                 
    printf("Elapsed time: %.0f ns (%.9f s)\n",
           elapsed * 1e9, elapsed);

    for (int i = 0; (i < num_pages && i < 16); i++)
        printf("%c ", addr[i * page_size]);
    printf("\n");

    munmap(addr, page_size * num_pages);
    return 0;
}
