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
    const size_t off_child = 0;
    const size_t off_parent = 4096;

    int fd = open(path, O_RDWR);
    if (fd == -1) die("open");

    void *mmaped_ptr = mmap(NULL, map_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mmaped_ptr == MAP_FAILED) die("mmap");

    char *p = (char *)mmaped_ptr;
    pid_t pid = fork();
    if (pid == -1) die("fork");

    if (pid == 0) {
        // Child process
        printf("Child process (pid=%ld); mmap address: %p\n", (long)getpid(), mmaped_ptr);
        memcpy(p + off_child, "01234", 5); 
        sleep(1);                          
        char buf[6];
        memcpy(buf, p + off_parent, 5);
        buf[5] = '\0';
        printf("Child process (pid=%ld); read from mmaped_ptr[4096]: %s\n", (long)getpid(), buf);
        munmap(mmaped_ptr, map_len);
        close(fd);
        _exit(0);
    } else {
        // Parent process
        printf("Parent process (pid=%ld); mmap address: %p\n", (long)getpid(), mmaped_ptr);
        memcpy(p + off_parent, "56789", 5); 
        sleep(1);                          
        char buf[6];
        memcpy(buf, p + off_child, 5);
        buf[5] = '\0';
        printf("Parent process (pid=%ld); read from mmaped_ptr[0]: %s\n", (long)getpid(), buf);
        waitpid(pid, NULL, 0);
        munmap(mmaped_ptr, map_len);
        close(fd);
    }
    return 0;
}