#!/usr/bin/python3
# SPDX-FileCopyrightText: 2025 Abdelrahman Alhanbali <abdelrahman.alhanbali@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause
import librosa
import numpy as np

def extract_mfcc(input_wav_path, output_txt_path, dim=15):
    # 音声読み込み
    y, sr = librosa.load(input_wav_path, sr=16000)

    # 無音部分を除去
    y_trimmed, _ = librosa.effects.trim(y, top_db=30)  # はしきい値

    # MFCC 抽出
    mfcc = librosa.feature.mfcc(y=y_trimmed, sr=sr, n_mfcc=dim)

    # 転置して [frame][dimension]
    mfcc = mfcc.T

    # 出力
    with open(output_txt_path, 'w', encoding='utf-8') as f:
        f.write(output_txt_path.split('/')[-1].replace('.txt','') + '\n')  
        f.write('ZAIMOKUCHOO\n')  
        f.write(str(mfcc.shape[0]) + '\n')
        for frame in mfcc:
            f.write(' '.join(f"{x:.6f}" for x in frame) + '\n')

    print(f"保存完了: {output_txt_path}({mfcc.shape[0]}フレーム）")
extract_mfcc("ZAIMOKUCHOO.wav", "city031_050.txt")
