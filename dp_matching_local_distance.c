// SPDX-FileCopyrightText: 20255555elrahman Alhanbali <abdelrahman.alhanbali@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_FRAME 200
#define DIM 15
#define N_WORDS 100

double template[MAX_FRAME][DIM];
double input[MAX_FRAME][DIM];
double g[MAX_FRAME][MAX_FRAME];
char buf[256];

int read_data(const char *filename, double data[MAX_FRAME][DIM], int *frame_count) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return -1;

    fgets(buf, sizeof(buf), fp);
    fgets(buf, sizeof(buf), fp);
    fgets(buf, sizeof(buf), fp);
    *frame_count = atoi(buf);

    for (int i = 0; i < *frame_count; i++) {
        for (int j = 0; j < DIM; j++) {
            if (fscanf(fp, "%lf", &data[i][j]) != 1) {
                fclose(fp);
                return -1;
            }
        }
    }

    fclose(fp);
    return 0;
}

double local_distance(double *a, double *b) {
    double sum = 0.0;
    for (int k = 0; k < DIM; k++) {
        double d = a[k] - b[k];
        sum += d * d;
    }
    return sqrt(sum);
}

double dp_matching(int frameA, int frameB, double diag_weight) {
    g[0][0] = local_distance(template[0], input[0]);

    for (int i = 1; i < frameA; i++)
        g[i][0] = g[i - 1][0] + local_distance(template[i], input[0]);
    for (int j = 1; j < frameB; j++)
        g[0][j] = g[0][j - 1] + local_distance(template[0], input[j]);

    for (int i = 1; i < frameA; i++) {
        for (int j = 1; j < frameB; j++) {
            double d = local_distance(template[i], input[j]);
            g[i][j] = fmin(fmin(g[i-1][j] + d,
                                g[i][j-1] + d),
                           g[i-1][j-1] + diag_weight * d);
        }
    }

    return g[frameA - 1][frameB - 1] / (frameA + frameB);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "使い方: %s <template_dir> <input_dir>\n", argv[0]);
        return 1;
    }

    const char *template_dir = argv[1];
    const char *input_dir = argv[2];

    double weights[] = {0.5, 1.0, 2.0};
    int num_weights = sizeof(weights) / sizeof(weights[0]);

    for (int w = 0; w < num_weights; w++) {
        double weight = weights[w];
        char filename[256];
        snprintf(filename, sizeof(filename), "result_weight_%.1f.csv", weight);
        FILE *csv = fopen(filename, "w");
        if (!csv) {
            fprintf(stderr, "CSVファイル %s 作成に失敗しました。\n", filename);
            continue;
        }

        fprintf(csv, "\xEF\xBB\xBF");
        fprintf(csv, "単語番号,認識結果,正誤,最小距離\n");

        int correct = 0, total = 0;
        double global_min_dist = 1e9;

        for (int n = 1; n <= N_WORDS; n++) {
            char input_path[256];
            snprintf(input_path, sizeof(input_path), "%s/city%03d_%03d.txt", input_dir, atoi(&input_dir[4]), n);

            int frameB;
            if (read_data(input_path, input, &frameB) != 0) continue;

            double min_dist = 1e9;
            int min_index = -1;

            for (int m = 1; m <= N_WORDS; m++) {
                char tmpl_path[256];
                snprintf(tmpl_path, sizeof(tmpl_path), "%s/city%03d_%03d.txt", template_dir, atoi(&template_dir[4]), m);

                int frameA;
                if (read_data(tmpl_path, template, &frameA) != 0) continue;

                double dist = dp_matching(frameA, frameB, weight);
                if (dist < min_dist) {
                    min_dist = dist;
                    min_index = m;
                }
            }

            if (min_dist < global_min_dist)
                global_min_dist = min_dist;

            int is_correct = (min_index == n);
            if (is_correct) correct++;
            total++;

            fprintf(csv, "%03d,%03d,%s,%.6f\n", n, min_index, is_correct ? "〇" : "×", min_dist);
        }

        double rate = 100.0 * correct / total;
        printf("[重み %.1f] 最小累積距離: %.6f 認識率: %.2f%%\n", weight, global_min_dist, rate);
        fclose(csv);
    }

    return 0;
}
