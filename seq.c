#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

typedef struct {
    double x, y, z;
} Point;

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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        printf("File open error\n");
        return 1;
    }

    int count = 0;
    char line[256];
    while (fgets(line, sizeof(line), f)) count++;
    rewind(f);

    Point *points = malloc(count * sizeof(Point));
    int N = 0;
    while (fgets(line, sizeof(line), f) && N < count) {
        if (sscanf(line, "%lf %lf %lf", &points[N].x, &points[N].y, &points[N].z) == 3) {
            N++;
        }
    }
    fclose(f);

    printf("Loaded %d points.\n", N);

    double max_area = 0.0;
    int best_i = -1, best_j = -1, best_k = -1;

    double t0 = GetTime();

    for (int i = 0; i < N - 2; i++) {
        for (int j = i + 1; j < N - 1; j++) {
            for (int k = j + 1; k < N; k++) {
                double area = triangle_area(points[i], points[j], points[k]);
                if (area > max_area) {
                    max_area = area;
                    best_i = i; best_j = j; best_k = k;
                }
            }
        }
    }

    double t = GetTime() - t0;

    printf("Max Area: %.4f\n", max_area);
    printf("Points: [%d], [%d], [%d]\n", best_i, best_j, best_k);
    printf("Time: %.2f ms\n", t);

    free(points);
    return 0;
}