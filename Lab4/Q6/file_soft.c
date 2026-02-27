#include <unistd.h>
#include <stdio.h>    
#include <errno.h>

int main(void) {
    if (symlink("file_original.txt", "file_soft.txt") == -1) {
        perror("symlink failed");
        return 1;
    }
    return 0;
}
