#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

void timestamp()
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("%d:%02d:%02d - ", t->tm_hour, t->tm_min, t->tm_sec);
}

int main()
{
    const char *name = "/tmp/myfifo";
    const char *msg = "hello";
    int fd;

    int mk = mkfifo(name, 0666);
    printf("%d\n", mk);
    sleep(1);

    fd = open(name, O_WRONLY);
    timestamp();
    printf("open is completed\n");
    write(fd, msg, strlen(msg) + 1);
    timestamp();
    printf("write is completed\n");
    sleep(1);
    close(fd);
    timestamp();
    printf("close is completed\n");
    sleep(1);

    unlink(name);
    timestamp();
    printf("unlink is completed\n");
    sleep(1);

    return 0;
}