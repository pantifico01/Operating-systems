#include <fcntl.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define SHM_SIZE 4096
static const char CHILD_NAME[] = "child";

static int join_path(char *buf, int size, const char *dir, const char *name) {
    int k = 0;
    for (int i = 0; dir[i] && k < size - 1; ++i) buf[k++] = dir[i];
    if (k < size - 1) buf[k++] = '/';
    for (int i = 0; name[i] && k < size - 1; ++i) buf[k++] = name[i];
    buf[k] = '\0';
    return k;
}

int main(void) {
    char out1[256];
    char out2[256];

    int n1 = read(0, out1, 255);
    int n2 = read(0, out2, 255);
    if (n1 <= 0 || n2 <= 0) {
        write(2, "Error\n", 6);
        exit(-1);
    }
    if (n1 > 0 && out1[n1 - 1] == '\n') out1[n1 - 1] = '\0'; else out1[n1] = '\0';
    if (n2 > 0 && out2[n2 - 1] == '\n') out2[n2 - 1] = '\0'; else out2[n2] = '\0';

    char progpath[1024];
    ssize_t len = readlink("/proc/self/exe", progpath, sizeof(progpath) - 1);
    if (len == -1) { write(2, "Error\n", 6); exit(-1); }
    progpath[len] = '\0';
    while (len > 0 && progpath[len] != '/') --len;
    progpath[len] = '\0';

    char path_child[1024];
    join_path(path_child, sizeof(path_child), progpath, CHILD_NAME);

    char shm_name[64];
    char sem_empty_name[64];
    char sem_full1_name[64];
    char sem_full2_name[64];
    snprintf(shm_name, sizeof(shm_name), "/shm_lab3_%d", (int)getpid());
    snprintf(sem_empty_name, sizeof(sem_empty_name), "/sem_empty_%d", (int)getpid());
    snprintf(sem_full1_name, sizeof(sem_full1_name), "/sem_full1_%d", (int)getpid());
    snprintf(sem_full2_name, sizeof(sem_full2_name), "/sem_full2_%d", (int)getpid());

    int shm = shm_open(shm_name, O_CREAT | O_RDWR, 0600);
    if (shm == -1) { write(2, "Error\n", 6); exit(-1); }
    if (ftruncate(shm, SHM_SIZE) == -1) { write(2, "Error\n", 6); exit(-1); }
    
    char *shm_buf = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    if (shm_buf == MAP_FAILED) { write(2, "Error\n", 6); exit(-1); }

    uint32_t *shm_length = (uint32_t *)shm_buf;
    char *shm_text = shm_buf + sizeof(uint32_t);

    sem_t *sem_empty = sem_open(sem_empty_name, O_CREAT, 0600, 1);
    sem_t *sem_full1 = sem_open(sem_full1_name, O_CREAT, 0600, 0);
    sem_t *sem_full2 = sem_open(sem_full2_name, O_CREAT, 0600, 0);
    if (sem_empty == SEM_FAILED || sem_full1 == SEM_FAILED || sem_full2 == SEM_FAILED) {
        write(2, "Error\n", 6);
        exit(-1);
    }

    pid_t child1 = fork();
    if (child1 == -1) { write(2, "Error\n", 6); exit(-1); }
    else if (child1 == 0) {
        char *args[] = {(char *)CHILD_NAME, shm_name, sem_full1_name, sem_empty_name, out1, NULL};
        execv(path_child, args);
        write(2, "Error\n", 6); exit(-1);
    }

    pid_t child2 = fork();
    if (child2 == -1) { write(2, "Error\n", 6); exit(-1); }
    else if (child2 == 0) {
        char *args[] = {(char *)CHILD_NAME, shm_name, sem_full2_name, sem_empty_name, out2, NULL};
        execv(path_child, args);
        write(2, "Error\n", 6); exit(-1);
    }

    char buffer[2048];
    char line[1024];
    int ind = 0;
    int bytes;

    while (1) {
        bytes = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (bytes == -1) { write(2, "Error\n", 6); exit(-1); }
        if (bytes == 0) break;

        for (int i = 0; i < bytes; ++i) {
            char c = buffer[i];
            if (c == '\n') {
                line[ind] = '\0';
                sem_wait(sem_empty);
                *shm_length = ind;
                memcpy(shm_text, line, ind);
                
                if (ind > 10) {
                    sem_post(sem_full2);
                } else {
                    sem_post(sem_full1);
                }
                ind = 0;
            } else {
                if (ind < (int)sizeof(line) - 1) line[ind++] = c;
            }
        }
    }

    if (ind > 0) {
        line[ind] = '\0';
        sem_wait(sem_empty);
        *shm_length = ind;
        memcpy(shm_text, line, ind);
        if (ind > 10) sem_post(sem_full2); else sem_post(sem_full1);
    }

    sem_wait(sem_empty); *shm_length = UINT32_MAX;
    sem_post(sem_full1); sem_post(sem_full2);

    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);

    sem_close(sem_empty); sem_close(sem_full1); sem_close(sem_full2);
    sem_unlink(sem_empty_name); sem_unlink(sem_full1_name); sem_unlink(sem_full2_name);
    munmap(shm_buf, SHM_SIZE); close(shm); shm_unlink(shm_name);
    return 0;
}