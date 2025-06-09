#include <memory.h>

#include <cfloat>
#include <cstdio>
#include <cstdlib>
#include <random>

#include "../head/save.h"
using namespace std;

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

constexpr long long STORAGE_FREQUENCY = (5 * 10000000);
constexpr long long STORAGE_COUNT = 1;
constexpr long long MAX_GAMES = 1000000000;

vector<string> Exploratorys = {"greedy", "ε-GREEDY", "UCB"};
int TupleNumber = 0;
int Multistaging = 1;
int OI = 1200;
int TC = 1;
string Exploratory = "GREEDY";
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
  // TC = atoi(argv[4]);
  // Exploratory = argv[5];
  int tmp1 = atoi(argv[4]);
  string tmp2 = argv[5];
  Restart_Len = atoi(argv[6]);
  int seed = atoi(argv[7]);
  srand(seed);
  init(TupleNumber, Multistaging);
  print_info();
  initEvs(OI);

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
      // 手の学習
      state_t copy, train_state;
      double qs[4];
      int rs[4];
      double ns[4];
      double max_q = -DBL_MAX;
      int train = -1;
      double sum_n = 1;
      int mincounts[4];
      int sum_mincount = 0;
      double minAerr[4];
      double sum_minAerr = 0;
      for (int d = 0; d < 4; d++) {
        if (play(d, state, &copy)) {
          qs[d] = calcEv(copy.board);
          rs[d] = copy.score - state.score;
          ns[d] = calcAErr(copy.board);
          if (ns[d] < 1e-12) ns[d] = 1e-12;
          if (max_q < qs[d] + rs[d]) {
            max_q = qs[d] + rs[d];
            train = d;
          }
          sum_n += ns[d];
          mincounts[d] = calcMinCount(copy.board);
          sum_mincount += mincounts[d];
          minAerr[d] = calcMinAErr(copy.board);
          sum_minAerr += minAerr[d];
          // printf("%d %f %f\n", d, qs[d], ns[d]);
        } else {
          qs[d] = rs[d] = ns[d] = -1;
        }
      }

      // 1手先を作って学習
      play(train, state, &train_state);
      double train_err = calcErr(train_state.board);
      double train_aerr = calcAErr(train_state.board);
      int train_board[9];
      for (int i = 0; i < 9; i++) {
        train_board[i] = train_state.board[i];
      }
      putNewTile(&train_state);
      if (gameOver(train_state)) {
        traincount++;
        if (traincount % STORAGE_FREQUENCY == 0)
          saveEvs(seed, TupleNumber, storage_c);
      } else {
        double max_ev_r = -DBL_MAX;
        for (int d = 0; d < 4; d++) {
          if (play(d, train_state, &copy)) {
            double ev_r = calcEv(copy.board) + (copy.score - train_state.score);
            if (ev_r > max_ev_r) max_ev_r = ev_r;
          }
        }
        traincount++;
        if (traincount % STORAGE_FREQUENCY == 0)
          saveEvs(seed, TupleNumber, storage_c);
      }

      // ε-Greedy にて移動
      int best = -1;
      int others[3];
      int others_count = 0;
      double max_v = -DBL_MAX;
      for (int d = 0; d < 4; d++) {
        if (ns[d] == -1) continue;
        double v = qs[d] + rs[d];
        if (max_v < v) {
          if (best != -1) {
            others[others_count++] = best;
          }
          max_v = v;
          best = d;
        } else {
          others[others_count++] = d;
        }
      }
      double r = rand() / (RAND_MAX + 1.0);
      double p = (double)traincount / STORAGE_FREQUENCY / STORAGE_COUNT;
      int selected;
      if (r < 0.9 + p / 9 || others_count == 0) {
        // if (r < 0.9 + 0.1 * p || others_count == 0) {
        // if (r < 0.95 + 0.05 * p || others_count == 0) {
        selected = best;
      } else {
        selected = others[rand() % others_count];
      }

      // 選択した手の順位を保存
      int rank = 0;
      for (int d = 0; d < 4; d++) {
        if (qs[d] + rs[d] > qs[selected] + rs[selected]) rank++;
      }

      // state.print();
      // printf("selected = %d\n", selected);
      if (selected == -1) {
        fprintf(stderr, "Something wrong. No direction played.\n");
      }
      play(selected, state, &state);
      putNewTile(&state);

      if (gameOver(state)) {
        traincount++;
        if (traincount % STORAGE_FREQUENCY == 0)
          saveEvs(seed, TupleNumber, storage_c);
        // printf("game %lld finished with score %d\n", gid + 1, state.score);
        if (gid % 100 == 0) {
          printf("game %lld traincount %lld finished with score %d\n", gid + 1,
                 traincount, state.score);
        }
        if (Restart_Len == -1) {
          break;
        }
        if (restart_count == 0)
          printf("game %lld finished with score %d (no restart)\n", gid + 1,
                 state.score);
        restart_points[restart_count++] = turn;
        if (turn - restart_start <= Restart_Len) {
          printf("restart end: %d subgames until turns", restart_count);
          for (int i = 0; i < restart_count; i++)
            printf(" %d", restart_points[i]);
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
  return 0;
}
