#!/bin/bash

# NT1〜NT9までループ
for i in {1..9}; do
    DIR="NT${i}/learning"                # 実行ファイルとTUPLEファイルのディレクトリ
    TUPLE_FILE="NT${i}_tuple_list_mode1" # 対応するファイル

    if [ -f "$TUPLE_FILE" ]; then
        echo "読み込み中のファイル: $TUPLE_FILE"

        # TUPLE_NUMBERSを取得（空白または改行で区切られた値を配列に格納）
        TUPLE_NUMBERS=($(cat "$TUPLE_FILE"))

        # 入力値のパラメータ
        MULTISTAGING_VALUES=(2)               # Multistaging の値
        OI_VALUES=(0 1200 2400 3600 4800)       # OI の値
        RESTART_LEN_VALUES=(1000) # Restart_Len の値
        SEEDS=(0)                               # seed の値

        # 各実行ファイルに対してループ
        for EXECUTABLE in "$DIR"/NT${i}_*; do
            if [ -x "$EXECUTABLE" ]; then
                echo "実行中のプログラム: $EXECUTABLE"

                # 各パラメータの組み合わせでループ
                for TUPLE_NUMBER in "${TUPLE_NUMBERS[@]}"; do
                    for MULTISTAGING in "${MULTISTAGING_VALUES[@]}"; do
                        for OI in "${OI_VALUES[@]}"; do
                            for RESTART_LEN in "${RESTART_LEN_VALUES[@]}"; do
                                for SEED in "${SEEDS[@]}"; do
                                    # Exploratory と TC の仮の値
                                    TC=1
                                    EXPLORATORY=1

                                    # 実行コマンド
                                    echo "実行中: $EXECUTABLE $TUPLE_NUMBER $MULTISTAGING $OI $TC $EXPLORATORY $RESTART_LEN $SEED"
                                    "$EXECUTABLE" "$TUPLE_NUMBER" "$MULTISTAGING" "$OI" "$TC" "$EXPLORATORY" "$RESTART_LEN" "$SEED"

                                    # エラーチェック
                                    if [ $? -ne 0 ]; then
                                        echo "エラー: $EXECUTABLE の実行中に失敗しました" >&2
                                        exit 1
                                    fi
                                done
                            done
                        done
                    done
                done | tail -n +3
            else
                echo "スキップ: $EXECUTABLE は実行可能ファイルではありません"
            fi
        done
    else
        echo "スキップ: $TUPLE_FILE は存在しません"
    fi
done

echo "すべてのプログラムの実行が完了しました。"
