// SPDX-FileCopyrightText: 2025 Abdelrahman Alhanbali <abdelrahman.alhanbali@gmail.com>
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
    if (!fp) {
        return -1;
    }

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

double dp_matching(int frameA, int frameB) {
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
                           g[i-1][j-1] + 2 * d);
        }
    }

    return g[frameA - 1][frameB - 1] / (frameA + frameB);
}

int main(int argc, char *argv[]) {
    const char *template_dir = argv[1];
    const char *input_dir = argv[2];

    char tmpl_path[256], input_path[256];
    int correct = 0, total = 0;
    double global_min_dist = 1e9;

    FILE *csv = fopen("recognition_result.csv", "w");
    if (!csv) {
        fprintf(stderr, "CSVファイル作成に失敗しました。\n");
        return 1;
    }

    // UTF-8 BOMを出力してExcelで文字化けを防止
    fprintf(csv, "\xEF\xBB\xBF");

    // ヘッダを日本語で出力
    fprintf(csv, "単語番号,認識結果,正誤,最小距離\n");


    for (int n = 1; n <= N_WORDS; n++) {
        snprintf(input_path, sizeof(input_path), "%s/city%03d_%03d.txt", input_dir, atoi(&input_dir[4]), n);

        int frameB;
        if (read_data(input_path, input, &frameB) != 0) {
            continue;
        }

        double min_dist = 1e9;
        int min_index = -1;

        for (int m = 1; m <= N_WORDS; m++) {
            snprintf(tmpl_path, sizeof(tmpl_path), "%s/city%03d_%03d.txt", template_dir, atoi(&template_dir[4]), m);

            int frameA;
            if (read_data(tmpl_path, template, &frameA) != 0) {
                continue;
            }

            double dist = dp_matching(frameA, frameB);
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

    printf("最小累積距離: %.6f\n", global_min_dist);
    printf("認識率: %.2f %%\n", rate);
    printf("認識結果を recognition_result.csv に保存。\n");

    fclose(csv);
    return 0;
}
