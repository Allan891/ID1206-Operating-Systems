#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    const char *path = (argc > 1) ? argv[1] : "file_to_map.txt";
    const size_t map_len = 1024 * 1024;  
    const size_t off_child_write = 0;
    const size_t off_parent_write = 4096; 

    
    setvbuf(stdout, NULL, _IONBF, 0);

    int fd = open(path, O_RDWR);
    if (fd == -1) {
        die("open");
    }

    //Avoid faults when accessing beyond EOF.
    struct stat st;
    if (fstat(fd, &st) == -1) die("fstat");
    if ((size_t)st.st_size < map_len) {
        fprintf(stderr,
                "ERROR: '%s' is %lld bytes; expected at least %zu bytes.\n"
                "Create it first: truncate -s 1M %s\n",
                path, (long long)st.st_size, map_len, path);
        close(fd);
        return EXIT_FAILURE;
    }

    // Two pipes /two way handshake:
    int c2p[2], p2c[2];
    if (pipe(c2p) == -1) die("pipe c2p");
    if (pipe(p2c) == -1) die("pipe p2c");
    
    pid_t pid = fork();
    if (pid == -1) die("fork"); 

    // Parent and child call mmap() themselves with MAP_SHARED.
    void *mmaped_ptr = mmap(NULL, map_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mmaped_ptr == MAP_FAILED) die("mmap");
    char *p = (char *)mmaped_ptr;

    if (pid == 0) { 
        close(c2p[0]); 
        close(p2c[1]); 

        printf("Child process (pid=%ld); mmap address: %p\n", (long)getpid(), mmaped_ptr);

        memcpy(p + off_child_write, "01234", 5);

        if (msync(mmaped_ptr, map_len, MS_SYNC) == -1) die("msync(child)");

        // Child finished writing
        if (write(c2p[1], "C", 1) != 1) die("write c2p");

        // Wait until parent to finish writing
        char token;
        if (read(p2c[0], &token, 1) != 1) die("read p2c");

        char buf[6];
        memcpy(buf, p + off_parent_write, 5);
        buf[5] = '\0';
        printf("Child process (pid=%ld); read from mmaped_ptr[4096]: %s\n", (long)getpid(), buf);

        munmap(mmaped_ptr, map_len);
        close(c2p[1]);
        close(p2c[0]);
        close(fd);
        _exit(0);
    } else {
        close(c2p[1]);
        close(p2c[0]);

        printf("Parent process (pid=%ld); mmap address: %p\n", (long)getpid(), mmaped_ptr);
        
        char token;
        if (read(c2p[0], &token, 1) != 1) die("read c2p");

        memcpy(p + off_parent_write, "56789", 5);

        if (msync(mmaped_ptr, map_len, MS_SYNC) == -1) die("msync(parent)");

        // Parent reads 5 chars from mmaped_ptr[0]
        char buf[6];
        memcpy(buf, p + off_child_write, 5);
        buf[5] = '\0';
        printf("Parent process (pid=%ld); read from mmaped_ptr[0]: %s\n", (long)getpid(), buf);

        //parent finished writing
        if (write(p2c[1], "P", 1) != 1) die("write p2c");

        int status = 0;
        waitpid(pid, &status, 0);

        munmap(mmaped_ptr, map_len);
        close(c2p[0]);
        close(p2c[1]);
        close(fd);

        return (WIFEXITED(status) ? WEXITSTATUS(status) : EXIT_FAILURE);
    }
}