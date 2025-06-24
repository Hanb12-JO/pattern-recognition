import os
import glob
import csv
import matplotlib.pyplot as plt

def draw_path(csv_path, save_dir=None):
    path_i = []
    path_j = []

    with open(csv_path, 'r') as f:
        reader = csv.reader(f)
        for row in reader:
            if len(row) != 2:
                continue
            try:
                i, j = int(row[0]), int(row[1])
                path_i.append(i)
                path_j.append(j)
            except ValueError:
                continue

    if len(path_i) < 2:
        print(f"[スキップ] {csv_path}：点が足りません。")
        return

    # 描画
    plt.figure(figsize=(6, 6))
    plt.plot(path_j, path_i, marker='o', markersize=2, linestyle='-', color='blue')
    plt.xlabel("Input Frame (j)")
    plt.ylabel("Template Frame (i)")
    plt.title(os.path.basename(csv_path))
    plt.gca().invert_yaxis()
    plt.grid(True)
    plt.tight_layout()

    # 保存先フォルダ作成
    if save_dir:
        os.makedirs(save_dir, exist_ok=True)
        base = os.path.splitext(os.path.basename(csv_path))[0]
        save_path = os.path.join(save_dir, f"{base}.png")
        plt.savefig(save_path, dpi=300)
        print(f"[保存] {save_path}")
        plt.close()
    else:
        plt.show()

def draw_all_paths(csv_folder, save_dir):
    csv_files = sorted(glob.glob(os.path.join(csv_folder, "path_word*.csv")))
    if not csv_files:
        print("指定フォルダにCSVファイルが見つかりません。")
        return

    for csv_path in csv_files:
        draw_path(csv_path, save_dir)

if __name__ == "__main__":
    import sys
    if len(sys.argv) < 3:
        print("使い方: python draw_all_paths.py <csvフォルダ> <画像保存フォルダ>")
        sys.exit(1)

    csv_folder = sys.argv[1]
    image_folder = sys.argv[2]
    draw_all_paths(csv_folder, image_folder)
