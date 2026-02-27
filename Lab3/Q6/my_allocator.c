#include <stdio.h> 
#include <stdlib.h>    
#include <unistd.h>  
#include <stdint.h>
#include <string.h>


int main(void) {
    int page_size;          
    int num_pages;    
    size_t total_bytes;     
    unsigned char *buffer;

    //Get the system-specific page size.   
    page_size = getpagesize();
    printf("System page size: %d bytes\n", page_size);


    // nr. of pages to allocate.
    printf("Enter number of pages to allocate: ");
    if (scanf("%d", &num_pages) != 1) {
        fprintf(stderr, "Failed to read number of pages.\n");
        return EXIT_FAILURE;
    }

    if (num_pages <= 0) {
        fprintf(stderr, "Number of pages must be positive.\n");
        return EXIT_FAILURE;
    }

    // total of bytes.
    if ((unsigned int)num_pages > (unsigned int)(SIZE_MAX / page_size)) {
        fprintf(stderr, "Requested size is too large and would overflow.\n");
        return EXIT_FAILURE;
    }

    total_bytes = (size_t)num_pages * (size_t)page_size;
    printf("Allocating %zu bytes (%d pages x %d bytes/page)\n",
           total_bytes, num_pages, page_size);

    // Allocate the memory, malloc blocked
    buffer = (unsigned char *)malloc(total_bytes);
    if (buffer == NULL) {
        perror("malloc failed");
        return EXIT_FAILURE;
    }

    printf("Memory allocation successful. Buffer address: %p\n", (void *)buffer);
    // memset, starting at the first n bytes 
    memset(buffer, 0, total_bytes);
    printf("Memory initialized to zero using memset().\n");


    // free the allocated memory.
    free(buffer);
    printf("Memory freed. Program exiting.\n");

    return EXIT_SUCCESS;
}
