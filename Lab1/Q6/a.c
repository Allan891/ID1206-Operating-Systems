#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

#define BUFSIZE 1024

void timestamp()
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("%d:%02d:%02d - ", t->tm_hour, t->tm_min, t->tm_sec);
}

int main () {
    const char *name = "/tmp/myfifo";
    char recv_buf[BUFSIZE];

    int fd;

    int mk = mkfifo(name, 0666);
    printf("%d\n", mk);
    sleep(3);

    fd = open(name, O_RDONLY);
    timestamp();
    printf("open is completed\n");
    sleep(2);
    read(fd, recv_buf, BUFSIZE);
    timestamp();
    printf("read is completed\n");
    sleep(2);
    close(fd);
    timestamp();
    printf("close is completed\n");
    sleep(2);

    timestamp();
    printf("Recieved %s \n", recv_buf);
    return 0;
}