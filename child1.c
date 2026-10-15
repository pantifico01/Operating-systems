#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

ssize_t read_line(int fd, char *buf, size_t max) {
    size_t i = 0;
    while (i < max - 1) {
        char c;
        ssize_t r = read(fd, &c, 1);
        if (r <= 0) {
            if (i == 0) {
                return -1;
            }
            break;
        }
        if (c == '\n') {
            break;
        }
        buf[i++] = c;
    }
    buf[i] = '\0';
    return (ssize_t)i;
}

void Invert(char *s, int len) {
    for (int i = 0; i < len / 2; ++i) {
        char tmp = s[i];
        s[i] = s[len - 1 - i];
        s[len - 1 - i] = tmp;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        write(2, "error\n", 6);
        exit(1);
    }

    int out = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out == -1) {
        write(2, "error of opening file\n", 22);
        exit(1);
    }

    if (dup2(out, 1) == -1) {
        write(2, "error dup2\n", 11);
        exit(1);
    }
    close(out);

    char line[1024];
    while (1) {
        ssize_t len = read_line(0, line, sizeof(line));
        if (len < 0) {
            break;
        }
        Invert(line, len);
        if (write(1, line, len) == -1) {
            write(2, "write error\n", 12);
            exit(1);
        }
        if (write(1, "\n", 1) == -1) {
            write(2, "write error\n", 12);
            exit(1);
        }
    }
    return 0;
}