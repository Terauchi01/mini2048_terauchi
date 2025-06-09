#include <cfloat>
#include <cstdio>
#include <cstdlib>
#include <random>

#include "../head/perfect_play.h"
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
#include "../head/4tuples_sym.h"
using namespace NT4;
const int nt = 4;
#endif

#include "../head/Game2048_3_3.h"

#define STORAGE_FREQUENCY (5 * 10000000)
#define STORAGE_COUNT 10
#define MAX_GAMES 1000000000

int storage_c = 0;

// void saveEvs(int seed, int TupleNumber) {
//   char filename[1024];
//   sprintf(filename,
//   "../dat/%dtuple_data_TupleNumber%d_seed%d_storage_c%d.dat",
//           nt, TupleNumber, seed, storage_c++);
//   FILE* fp = fopen(filename, "wb");
//   if (!fp) {
//     fprintf(stderr, "file %s open failed.\n", filename);
//   }
//   writeEvs(fp);
//   fclose(fp);
//   printf("stored %s\n", filename);
//   if (storage_c == STORAGE_COUNT) exit(0);
// }

int main(int argc, char* argv[]) {
  if (argc < 2 + 1) {
    fprintf(stderr, "Usage: learning_ntuple <seed> <TupleNumber>\n");
    exit(1);
  }
  readDB2();
  int seed = atoi(argv[1]);
  srand(seed);
  int TupleNumber = atoi(argv[2]);
  int Multistaging = 2;
  init(TupleNumber, Multistaging);

  initEvs(0);

  int traincount = 0;
  for (int gid = 0; gid < MAX_GAMES; gid++) {
    state_t state = initGame();
    int turn = 0;
    int lastboard[9] = {0};
    while (true) {  // ゲームのループ
      turn++;
      state_t copy;
      double max_ev_r = -DBL_MAX;
      int selected = -1;
      double ret[4];
      eval_afterstates(state, ret);
      for (int d = 0; d < 4; d++) {
        if (ret[d] > max_ev_r) {
          max_ev_r = ret[d];
          selected = d;
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
        if (traincount % STORAGE_FREQUENCY == 0) saveEvs(seed, TupleNumber);
      }
      for (int i = 0; i < 9; i++) {
        lastboard[i] = state.board[i];
      }
      putNewTile(&state);

      if (gameOver(state)) {
        update(lastboard, 0 - calcEv(lastboard));
        traincount++;
        if (traincount % STORAGE_FREQUENCY == 0) saveEvs(seed, TupleNumber);
        printf("game %d finished with score %d\n", gid + 1, state.score);
        break;
      }
    }
  }

  fprintf(stderr, "Training finished before saving %lld data\n", STORAGE_COUNT);
  return 0;
}
