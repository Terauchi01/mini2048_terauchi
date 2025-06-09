#!/bin/bash

# コンパイラとフラグを修正
CXX=clang++
CXXFLAGS="-O2 -std=c++20 -D_DARWIN_C_SOURCE"
# zlibのインクルードパスとライブラリパスを追加
CXXFLAGS+=" -I/opt/homebrew/include"
LDFLAGS="-L/opt/homebrew/opt/xz/lib -L/opt/homebrew/lib -lz"

# NT1〜NT9までループ
for i in {1..2}; do
    SRC_DIR="NT/learning"     # ソースコードがあるディレクトリ
    OUT_DIR="NT${i}/learning" # 出力先ディレクトリ

    # 出力ディレクトリが存在しない場合は作成
    mkdir -p "$OUT_DIR"

    if [ -d "$SRC_DIR" ]; then
        echo "処理中のディレクトリ: $SRC_DIR -> $OUT_DIR"

        # 各ファイルをコンパイル
        for FILE in learning_ntuple.cpp; do
            INPUT_FILE="$SRC_DIR/$FILE"
            BASE_NAME=$(basename "$FILE" .cpp)         # ファイル名から拡張子を除去
            OUTPUT_FILE="$OUT_DIR/NT${i}_${BASE_NAME}" # 実行ファイル名にディレクトリ情報を追加

            if [ -f "$INPUT_FILE" ]; then
                echo "コンパイル中: $INPUT_FILE -> $OUTPUT_FILE"
                $CXX $CXXFLAGS -D"T${i}" "$INPUT_FILE" -o "$OUTPUT_FILE" $LDFLAGS

                # コンパイル成功チェック
                if [ $? -eq 0 ]; then
                    echo "成功: $OUTPUT_FILE が生成されました"
                else
                    echo "エラー: $INPUT_FILE のコンパイルに失敗しました" >&2
                    exit 1
                fi
            else
                echo "スキップ: $INPUT_FILE は存在しません"
            fi
        done
    else
        echo "スキップ: $SRC_DIR ディレクトリが存在しません"
    fi
done

echo "すべての処理が完了しました。"
