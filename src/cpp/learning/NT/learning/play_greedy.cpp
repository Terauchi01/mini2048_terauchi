#include <zlib.h>

#include <cfloat>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <random>
using namespace std;

#include "../head/Game2048_3_3.h"
// #define NT4A
#if defined(T1)
#include "../head/1tuples_sym.h"
using namespace NT1;
int nt = 1;
#elif defined(T2)
#include "../head/2tuples_sym.h"
using namespace NT2;
int nt = 2;
#elif defined(T3)
#include "../head/3tuples_sym.h"
using namespace NT3;
int nt = 3;
#elif defined(T4)
#include "../head/4tuples_sym.h"
using namespace NT4;
int nt = 4;
#elif defined(T5)
#include "../head/5tuples_sym.h"
using namespace NT5;
int nt = 5;
#elif defined(T6)
#include "../head/6tuples_sym.h"
using namespace NT6;
int nt = 6;
#elif defined(T7)
#include "../head/7tuples_sym.h"
using namespace NT7;
int nt = 7;
#elif defined(T8)
#include "../head/8tuples_sym.h"
using namespace NT8;
int nt = 8;
#elif defined(T9)
#include "../head/9tuples_sym.h"
using namespace NT9;
int nt = 9;
#else
// デフォルトを NT6 に設定
#include "../head/1tuples_sym.h"
using namespace NT1;
int nt = 1;
#endif

// #define OUTPUT_AFTERSTATES

bool loadCompressedEvs(const char* filename) {
  printf("Debug: Opening compressed file: %s\n", filename);

  // NT1の初期化状態を確認
  printf("Debug: Before init - num_stages=%d, num_tuple=%d\n", num_stages,
         num_tuple);

  // NT1の初期化
  init(0, 2);

  printf("Debug: After init - Memory initialized:\n");
  printf("- evs: %p\n- errs: %p\n- aerrs: %p\n- updatecounts: %p\n", (void*)evs,
         (void*)errs, (void*)aerrs, (void*)updatecounts);

  gzFile gz_fp = gzopen(filename, "rb");
  if (!gz_fp) {
    fprintf(stderr, "Error opening compressed file\n");
    return false;
  }

  FILE* temp_fp = tmpfile();
  if (!temp_fp) {
    fprintf(stderr, "Error creating temporary file\n");
    gzclose(gz_fp);
    return false;
  }

  printf("Debug: Starting decompression\n");

  // 圧縮データを展開
  char buffer[4096];
  int bytes_read;
  size_t total_bytes = 0;

  while ((bytes_read = gzread(gz_fp, buffer, sizeof(buffer))) > 0) {
    size_t written = fwrite(buffer, 1, bytes_read, temp_fp);
    if (written != (size_t)bytes_read) {
      fprintf(stderr,
              "Error writing to temporary file: expected %d, wrote %zu\n",
              bytes_read, written);
      fclose(temp_fp);
      gzclose(gz_fp);
      return false;
    }
    total_bytes += written;
  }

  if (bytes_read < 0) {
    int errnum;
    fprintf(stderr, "Error during decompression: %s\n",
            gzerror(gz_fp, &errnum));
    fclose(temp_fp);
    gzclose(gz_fp);
    return false;
  }

  printf("Debug: Decompressed %zu bytes\n", total_bytes);

  // ファイルポインタを先頭に戻す
  rewind(temp_fp);

  // データを読み込む
  readEvs(temp_fp);

  // クリーンアップ
  fclose(temp_fp);
  gzclose(gz_fp);

  return true;
}

int main(int argc, char** argv) {
  if (argc < 3 + 1) {
    fprintf(stderr, "Usage: playgreedy <seed> <game_counts> <EV-file>\n");
    exit(1);
  }

  printf("Debug: Starting program\n");
  printf("Debug: Command line arguments:\n");
  printf("- seed: %s\n- game_counts: %s\n- EV-file: %s\n", argv[1], argv[2],
         argv[3]);

  int seed = atoi(argv[1]);
  int game_count = atoi(argv[2]);
  char* evfile = argv[3];

  // メモリ初期化前の確認
  printf("Debug: Checking NT1 initialization state\n");
  printf("Debug: num_stages=%d, num_tuple=%d, ARRAY_LENGTH=%lld\n", num_stages,
         num_tuple, ARRAY_LENGTH);

  srand(seed);
  printf("Debug: About to load compressed file\n");

  // ZIPファイルの存在確認
  FILE* test_fp = fopen(evfile, "rb");
  if (!test_fp) {
    fprintf(stderr, "Error: Cannot open file %s\n", evfile);
    exit(1);
  }
  fclose(test_fp);

  if (!loadCompressedEvs(evfile)) {
    fprintf(stderr, "Failed to load compressed file: %s\n", evfile);
    exit(1);
  }

  printf("Debug: File loaded successfully\n");
  printf("Debug: Memory state after loading:\n");
  printf("- evs: %p\n- errs: %p\n- aerrs: %p\n- updatecounts: %p\n", (void*)evs,
         (void*)errs, (void*)aerrs, (void*)updatecounts);

  for (int gid = 0; gid < game_count; gid++) {
    printf("Debug: Starting game %d\n", gid + 1);
    state_t state = initGame();
    int turn = 0;
    while (true) {
      turn++;
      state_t copy;
      double max_evr = -DBL_MAX;
      int selected = -1;
      for (int d = 0; d < 4; d++) {
        if (play(d, state, &copy)) {
          double evr = calcEv(copy.board) + (copy.score - state.score);
          if (evr > max_evr) {
            max_evr = evr;
            selected = d;
          }
        }
      }
      if (selected == -1) {
        fprintf(stderr, "Something wrong. No direction played.\n");
      }
      play(selected, state, &state);
#ifdef OUTPUT_AFTERSTATES
      for (int i = 0; i < 9; i++) {
        printf("%d ", state.board[i]);
      }
      printf("\n");
#endif
      putNewTile(&state);

      if (gameOver(state)) {
#ifndef OUTPUT_AFTERSTATES
        printf("game %d finished with score %d\n", gid + 1, state.score);
#endif
        break;
      }
    }
  }
}
