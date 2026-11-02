#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    double x, y, z;
} Point;

Point *points = NULL;
int N = 0;
double global_max_area = 0.0;
int best_i = -1, best_j = -1, best_k = -1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

double GetTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

double triangle_area(Point a, Point b, Point c) {
    double abx = b.x - a.x, aby = b.y - a.y, abz = b.z - a.z;
    double acx = c.x - a.x, acy = c.y - a.y, acz = c.z - a.z;
    double cx = aby * acz - abz * acy;
    double cy = abz * acx - abx * acz;
    double cz = abx * acy - aby * acx;
    return sqrt(cx * cx + cy * cy + cz * cz) / 2.0;
}

typedef struct {
    int thread_id;
    int total_threads;
    double local_max_area;
    int local_i, local_j, local_k;
} ThreadArg;

void *thread_func(void *arg) {
    ThreadArg *targ = (ThreadArg *)arg;
    targ->local_max_area = 0.0;

    int start_i = (N * targ->thread_id) / targ->total_threads;
    int end_i   = (N * (targ->thread_id + 1)) / targ->total_threads;

    for (int i = start_i; i < end_i && i < N - 2; i++) {
        for (int j = i + 1; j < N - 1; j++) {
            for (int k = j + 1; k < N; k++) {
                double area = triangle_area(points[i], points[j], points[k]);
                if (area > targ->local_max_area) {
                    targ->local_max_area = area;
                    targ->local_i = i;
                    targ->local_j = j;
                    targ->local_k = k;
                }
            }
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <threads> <file>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads < 1) num_threads = 1;

    FILE *f = fopen(argv[2], "r");
    if (!f) {
        printf("File open error\n");
        return 1;
    }

    int count = 0;
    char line[256];
    while (fgets(line, sizeof(line), f)) count++;
    
    points = malloc(count * sizeof(Point));
    rewind(f);
    while (fgets(line, sizeof(line), f) && N < count) {
        if (sscanf(line, "%lf %lf %lf", &points[N].x, &points[N].y, &points[N].z) == 3) {
            N++;
        }
    }
    fclose(f);

    printf("Loaded %d points. Threads: %d\n", N, num_threads);

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    ThreadArg *args = malloc(num_threads * sizeof(ThreadArg));

    double t0 = GetTime();

    for (int t = 0; t < num_threads; t++) {
        args[t].thread_id = t;
        args[t].total_threads = num_threads;
        args[t].local_max_area = 0.0;
        pthread_create(&threads[t], NULL, thread_func, &args[t]);
    }

    global_max_area = 0.0;
    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
        pthread_mutex_lock(&mutex);
        if (args[t].local_max_area > global_max_area) {
            global_max_area = args[t].local_max_area;
            best_i = args[t].local_i;
            best_j = args[t].local_j;
            best_k = args[t].local_k;
        }
        pthread_mutex_unlock(&mutex);
    }

    double t = GetTime() - t0;

    printf("Max Area: %.4f\n", global_max_area);
    printf("Points: [%d], [%d], [%d]\n", best_i, best_j, best_k);
    printf("Time: %.2f ms\n", t);

    free(points);
    free(threads);
    free(args);
    return 0;
}