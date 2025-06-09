#!/bin/bash

TUPLE_FILE="../../NT4_tuple_list_mode1.txt" # 対応するファイル

MAX_CONCURRENT_PROCESSES=32 # 最大並列プロセス数
PROCESS_COUNT=0             # 現在実行中のプロセス数

if [ -f "$TUPLE_FILE" ]; then
    echo "読み込み中のファイル: $TUPLE_FILE"

    # TUPLE_NUMBERSを取得（空白または改行で区切られた値を配列に格納）
    TUPLE_NUMBERS=($(cat "$TUPLE_FILE"))

    # 入力値のパラメータ
    MULTISTAGING_VALUES=(1 2)               # Multistaging の値
    OI_VALUES=(0 1200 2400 3600 4800)       # OI の値
    RESTART_LEN_VALUES=(10 50 100 500 1000) # Restart_Len の値
    SEEDS=(0)                               # seed の値

    # 各実行ファイルに対してループ
    for EXECUTABLE in NT4_*; do
        if [ -x "$EXECUTABLE" ]; then
            echo "実行中のプログラム: $EXECUTABLE"

            # 引数を設定
            UCB_C=1.2

            # EXECUTABLEのファイル名から"NT4_learning_ntuple"以降を取得
            EXECUTABLE_SUFFIX=$(echo "$EXECUTABLE" | sed 's/NT4_learning_ntuple//')

            # 各パラメータの組み合わせでループ
            for TUPLE_NUMBER in "${TUPLE_NUMBERS[@]}"; do
                for MULTISTAGING in "${MULTISTAGING_VALUES[@]}"; do
                    for OI in "${OI_VALUES[@]}"; do
                        for RESTART_LEN in "${RESTART_LEN_VALUES[@]}"; do
                            for SEED in "${SEEDS[@]}"; do
                                # Exploratory と TC の仮の値
                                TC=1
                                EXPLORATORY="test"

                                # 出力ファイル名を一意にする（SEEDを使ってファイル名に追加）
                                OUTPUT_FILE="output_${TUPLE_NUMBER}_${MULTISTAGING}_${OI}_${RESTART_LEN}_${SEED}_${EXECUTABLE_SUFFIX}.txt"

                                # 引数を設定（ucb2用）
                                if [[ "$EXECUTABLE" == *"ucb2"* ]]; then
                                    echo "実行中: ./$EXECUTABLE $TUPLE_NUMBER $MULTISTAGING $OI $TC $EXPLORATORY $RESTART_LEN $UCB_C $SEED  > $OUTPUT_FILE"
                                    nohup ./"$EXECUTABLE" "$TUPLE_NUMBER" "$MULTISTAGING" "$OI" "$TC" "$EXPLORATORY" "$RESTART_LEN" $UCB_C "$SEED" >"$OUTPUT_FILE" &
                                else
                                    # "ucb2" を含まない場合の引数設定
                                    echo "実行中: ./$EXECUTABLE $TUPLE_NUMBER $MULTISTAGING $OI $TC $EXPLORATORY $RESTART_LEN $SEED > $OUTPUT_FILE"
                                    nohup ./"$EXECUTABLE" "$TUPLE_NUMBER" "$MULTISTAGING" "$OI" "$TC" "$EXPLORATORY" "$RESTART_LEN" "$SEED" >"$OUTPUT_FILE" &
                                fi

                                # 実行中のプロセス数をカウント
                                PROCESS_COUNT=$((PROCESS_COUNT + 1))

                                # 最大並列プロセス数を超えた場合は待機
                                if [ "$PROCESS_COUNT" -ge "$MAX_CONCURRENT_PROCESSES" ]; then
                                    wait -n                              # 1つのプロセスが終わるまで待機
                                    PROCESS_COUNT=$((PROCESS_COUNT - 1)) # プロセスカウントを減らす
                                fi
                            done
                        done
                    done
                done
            done
        else
            echo "スキップ: $EXECUTABLE は実行可能ファイルではありません"
        fi
    done

    # すべてのバックグラウンドプロセスが終了するのを待つ
    wait
else
    echo "スキップ: $TUPLE_FILE は存在しません"
fi
