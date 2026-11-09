#include <fcntl.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_SIZE 4096

void Invert(char *s, int len) {
    for (int i = 0; i < len / 2; ++i) {
        char tmp = s[i];
        s[i] = s[len - 1 - i];
        s[len - 1 - i] = tmp;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        write(2, "Error\n", 6);
        exit(-1);
    }

    const char *shm_name = argv[1];
    const char *sem_full_name = argv[2];
    const char *sem_empty_name = argv[3];
    const char *out_filename = argv[4];

    int out_fd = open(out_filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (out_fd == -1) { write(2, "Error\n", 6); exit(-1); }

    int shm = shm_open(shm_name, O_RDWR, 0600);
    if (shm == -1) { write(2, "Error\n", 6); exit(-1); }

    char *shm_buf = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    if (shm_buf == MAP_FAILED) { write(2, "Error\n", 6); exit(-1); }

    uint32_t *length = (uint32_t *)shm_buf;
    char *text = shm_buf + sizeof(uint32_t);

    sem_t *sem_full = sem_open(sem_full_name, 0);
    sem_t *sem_empty = sem_open(sem_empty_name, 0);
    if (sem_full == SEM_FAILED || sem_empty == SEM_FAILED) {
        write(2, "Error\n", 6);
        exit(-1);
    }

    char result[SHM_SIZE];

    while (1) {
        sem_wait(sem_full);

        if (*length == UINT32_MAX) {
            sem_post(sem_empty);
            break;
        }

        int len = *length;
        memcpy(result, text, len);
        Invert(result, len);
        result[len] = '\n';
        ++len;

        if (write(out_fd, result, len) == -1) {
            write(2, "Error\n", 6);
            exit(-1);
        }
        
        sem_post(sem_empty);
    }

    sem_close(sem_full);
    sem_close(sem_empty);
    munmap(shm_buf, SHM_SIZE);
    close(shm);
    close(out_fd);
    return 0;
}