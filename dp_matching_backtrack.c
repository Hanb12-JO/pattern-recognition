// SPDX-FileCopyrightText: 2024 Abdelrahman Alhanbali <abdelrahman.alhanbali@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>  // フォルダ作成用（UNIX系）

#define MAX_FRAME 200
#define DIM 15
#define N_WORDS 100

double template[MAX_FRAME][DIM];
double input[MAX_FRAME][DIM];
double g[MAX_FRAME][MAX_FRAME];
int prev_i[MAX_FRAME][MAX_FRAME];
int prev_j[MAX_FRAME][MAX_FRAME];

int path_i[MAX_FRAME * 2];
int path_j[MAX_FRAME * 2];

char buf[256];

int read_data(const char *filename, double data[MAX_FRAME][DIM], int *frame_count) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return -1;

    fgets(buf, sizeof(buf), fp);  // ヘッダ1
    fgets(buf, sizeof(buf), fp);  // ヘッダ2
    fgets(buf, sizeof(buf), fp);  // フレーム数
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

double dp_matching_with_path(int frameA, int frameB, int *path_len) {
    g[0][0] = local_distance(template[0], input[0]);
    prev_i[0][0] = -1;
    prev_j[0][0] = -1;

    for (int i = 1; i < frameA; i++) {
        g[i][0] = g[i - 1][0] + local_distance(template[i], input[0]);
        prev_i[i][0] = i - 1;
        prev_j[i][0] = 0;
    }
    for (int j = 1; j < frameB; j++) {
        g[0][j] = g[0][j - 1] + local_distance(template[0], input[j]);
        prev_i[0][j] = 0;
        prev_j[0][j] = j - 1;
    }

    for (int i = 1; i < frameA; i++) {
        for (int j = 1; j < frameB; j++) {
            double d = local_distance(template[i], input[j]);
            double min_val = g[i - 1][j] + d;
            prev_i[i][j] = i - 1;
            prev_j[i][j] = j;

            if (g[i][j - 1] + d < min_val) {
                min_val = g[i][j - 1] + d;
                prev_i[i][j] = i;
                prev_j[i][j] = j - 1;
            }
            if (g[i - 1][j - 1] + 2 * d < min_val) {
                min_val = g[i - 1][j - 1] + 2 * d;
                prev_i[i][j] = i - 1;
                prev_j[i][j] = j - 1;
            }

            g[i][j] = min_val;
        }
    }

    int pi = frameA - 1, pj = frameB - 1, idx = 0;
    while (pi >= 0 && pj >= 0) {
        path_i[idx] = pi;
        path_j[idx] = pj;
        int ni = prev_i[pi][pj], nj = prev_j[pi][pj];
        idx++;
        if (ni == -1 && nj == -1) break;
        pi = ni;
        pj = nj;
    }

    for (int k = 0; k < idx / 2; k++) {
        int ti = path_i[k], tj = path_j[k];
        path_i[k] = path_i[idx - 1 - k];
        path_j[k] = path_j[idx - 1 - k];
        path_i[idx - 1 - k] = ti;
        path_j[idx - 1 - k] = tj;
    }

    *path_len = idx;
    return g[frameA - 1][frameB - 1] / (frameA + frameB);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "使い方: %s <template_dir> <input_dir>\n", argv[0]);
        return 1;
    }

    const char *template_dir = argv[1];
    const char *input_dir = argv[2];
    const char *path_dir = "paths";

    mkdir(path_dir, 0777);  // 出力フォルダ作成（既にある場合は無視）

    char tmpl_path[256], input_path[256];
    int correct = 0, total = 0;
    double global_min_dist = 1e9;

    FILE *csv = fopen("recognition_result_with_path.csv", "w");
    if (!csv) {
        fprintf(stderr, "CSVファイル作成に失敗しました。\n");
        return 1;
    }

    fprintf(csv, "\xEF\xBB\xBF");
    fprintf(csv, "単語番号,認識結果,正誤,最小距離,経路長\n");

    for (int n = 1; n <= N_WORDS; n++) {
        snprintf(input_path, sizeof(input_path), "%s/city%03d_%03d.txt", input_dir, atoi(&input_dir[4]), n);
        int frameB;
        if (read_data(input_path, input, &frameB) != 0) continue;

        double min_dist = 1e9;
        int min_index = -1;
        int best_path_len = 0;

        for (int m = 1; m <= N_WORDS; m++) {
            snprintf(tmpl_path, sizeof(tmpl_path), "%s/city%03d_%03d.txt", template_dir, atoi(&template_dir[4]), m);
            int frameA;
            if (read_data(tmpl_path, template, &frameA) != 0) continue;

            int path_len;
            double dist = dp_matching_with_path(frameA, frameB, &path_len);

            if (dist < min_dist) {
                min_dist = dist;
                min_index = m;
                best_path_len = path_len;
            }
        }

        if (min_dist < global_min_dist)
            global_min_dist = min_dist;

        int is_correct = (min_index == n);
        if (is_correct) correct++;
        total++;

        char path_file[256];
        snprintf(path_file, sizeof(path_file), "%s/path_word%03d.csv", path_dir, n);
        FILE *path_csv = fopen(path_file, "w");
        if (path_csv) {
            for (int i = 0; i < best_path_len; i++) {
                fprintf(path_csv, "%d,%d\n", path_i[i], path_j[i]);
            }
            fclose(path_csv);
        }

        fprintf(csv, "%03d,%03d,%s,%.6f,%d\n", n, min_index, is_correct ? "〇" : "×", min_dist, best_path_len);
    }

    double rate = 100.0 * correct / total;

    printf("最小累積距離: %.6f\n", global_min_dist);
    printf("認識率: %.2f %%\n", rate);
    printf("認識結果を recognition_result_with_path.csv に保存。\n");

    fclose(csv);
    return 0;
}
