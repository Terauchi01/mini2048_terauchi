#include <zlib.h>

#include <cfloat>
#include <cstdio>
#include <cstdlib>
#include <random>
using namespace std;

#include "../head/save.h"
#if defined(T1)
#include "../head/1tuples_sym.h"
using namespace NT1;
const int nt = 1;
#elif defined(T2)
#include "../head/2tuples_sym.h"
using namespace NT2;
const int nt = 2;
#elif defined(T3)
#include "../head/3tuples_sym.h"
using namespace NT3;
const int nt = 3;
#elif defined(T4)
#include "../head/4tuples_sym.h"
using namespace NT4;
const int nt = 4;
#elif defined(T5)
#include "../head/5tuples_sym.h"
using namespace NT5;
const int nt = 5;
#elif defined(T6)
#include "../head/6tuples_sym.h"
using namespace NT6;
const int nt = 6;
#elif defined(T7)
#include "../head/7tuples_sym.h"
using namespace NT7;
const int nt = 7;
#elif defined(T8)
#include "../head/8tuples_sym.h"
using namespace NT8;
const int nt = 8;
#elif defined(T9)
#include "../head/9tuples_sym.h"
using namespace NT9;
const int nt = 9;
#else
#include "../head/1tuples_sym.h"
using namespace NT1;
const int nt = 1;
#endif

#include "../head/Game2048_3_3.h"

constexpr long long STORAGE_FREQUENCY = (10 * 10000000);
constexpr long long STORAGE_COUNT = 1;
constexpr long long MAX_GAMES = 1000000000;

vector<string> Exploratorys = {"greedy", "ε-GREEDY", "UCB"};
int TupleNumber = 0;
int Multistaging = 1;
int OI = 1200;
int TC = 1;
string Exploratory = "greedy";
int Restart_Len = 1000;
int storage_c = 0;

int main(int argc, char* argv[]) {
  if (argc < 7 + 1) {
    fprintf(stderr,
            "Usage: learning_ntuple <TupleNumber> <Multistaging> <OI> <TC> "
            "<Exploratory> <Restart_Len> <seed> \n");
    exit(1);
  }
  TupleNumber = atoi(argv[1]);
  Multistaging = atoi(argv[2]);
  OI = atoi(argv[3]);
  TC = atoi(argv[4]);
  Exploratory = argv[5];
  Restart_Len = atoi(argv[6]);
  int seed = atoi(argv[7]);

  srand(seed);
  init(TupleNumber, Multistaging);
  print_info();

  // 以前のデータを読み込む（もし存在すれば）
  char filename[1024];
  snprintf(filename, sizeof(filename),
           "../dat/%dtuple_data_TupleNumber%d_seed%d_storage_c%d.dat.zip", nt,
           TupleNumber, seed, storage_c);

  if (FILE* test = fopen(filename, "r")) {
    fclose(test);
    // zipファイルが存在する場合、読み込みを試みる
    gzFile gz_fp = gzopen(filename, "rb");
    if (gz_fp) {
      FILE* temp_fp = tmpfile();
      if (temp_fp) {
        char buffer[4096];
        int bytes_read;
        while ((bytes_read = gzread(gz_fp, buffer, sizeof(buffer))) > 0) {
          fwrite(buffer, 1, bytes_read, temp_fp);
        }
        rewind(temp_fp);
        readEvs(temp_fp);
        fclose(temp_fp);
        printf("Loaded previous data from %s\n", filename);
      }
      gzclose(gz_fp);
    }
  } else {
    // 新規の場合は初期化
    initEvs(OI);
  }

  long long traincount = 0;
  for (long long gid = 0; gid < MAX_GAMES; gid++) {
    state_t state = initGame();
    int turn = 0;
    int lastboard[9] = {0};
    state_t saved_states[10000];
    int restart_points[100];
    int restart_count = 0;
    int restart_start = 0;
    while (true) {  // ゲームのループ
      saved_states[turn] = state.clone();
      turn++;
      state_t copy;
      double max_ev_r = -DBL_MAX;
      int selected = -1;
      for (int d = 0; d < 4; d++) {
        if (play(d, state, &copy)) {
          double ev_r = calcEv(copy.board) + (copy.score - state.score);
          if (ev_r > max_ev_r) {
            max_ev_r = ev_r;
            selected = d;
          }
          // printf("d=%d, ev_r=%f, max_ev_r=%f\n",
          // 	 d, ev_r, max_ev_r);
        }
      }
      // state.print();
      // printf("selected = %d\n", selected);
      if (selected == -1) {
        fprintf(stderr, "Something wrong. No direction played.\n");
      }
      play(selected, state, &state);
      if (turn > 1) {
        update(lastboard, max_ev_r - calcEv(lastboard));
        traincount++;
        if (traincount % STORAGE_FREQUENCY == 0) {
          saveEvs(seed, TupleNumber, storage_c);
          storage_c++;
        }
      }
      for (int i = 0; i < 9; i++) {
        lastboard[i] = state.board[i];
      }
      putNewTile(&state);

      if (gameOver(state)) {
        update(lastboard, 0 - calcEv(lastboard));
        traincount++;
        if (traincount % STORAGE_FREQUENCY == 0) {
          saveEvs(seed, TupleNumber, storage_c);
          storage_c++;
        }
        if (gid % 1 == 0) {
          printf("game %lld traincount %lld finished with score %d\n", gid + 1,
                 traincount, state.score);
        }
        if (Restart_Len == -1) {
          break;
        }
        if (restart_count == 0)
          // printf("game %lld finished with score %d (no restart)\n", gid + 1,
          //        state.score);
        restart_points[restart_count++] = turn;
        if (turn - restart_start <= Restart_Len) {
          // printf("restart end: %d subgames until turns", restart_count);
          for (int i = 0; i < restart_count; i++)
            // printf(" %d", restart_points[i]);
          printf("\n");
          break;
        } else {
          // リスタート
          turn = restart_start = (restart_start + turn) / 2;
          state = saved_states[restart_start];
        }
        break;
      }
    }
  }

  fprintf(stderr, "Training finished before saving %lld data\n", STORAGE_COUNT);
  cleanup();
  return 0;
}
