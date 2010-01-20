#include <PA9.h>
#include <fat.h>

#include "lua-5.1.4/src/lua.h"
#include "lua-5.1.4/src/lauxlib.h"
#include "lua-5.1.4/src/lualib.h"

#define SYSTEM_TIMER_CLOCK (2<<25)
#define TIMER_DIV TIMER_DIV_256
#define TIMER_CLOCK (SYSTEM_TIMER_CLOCK/256)

__attribute__((noinline)) static void PrfStart() {
  TIMER0_CR = 0;
  TIMER0_DATA = 0;
  TIMER0_CR = TIMER_ENABLE | TIMER_DIV;
  TIMER1_CR = 0;
  TIMER1_DATA = 0;
  TIMER1_CR = TIMER_ENABLE | TIMER_CASCADE;
}

__attribute__((noinline)) static u32 PrfEnd() {
  vu32 t0d = TIMER0_DATA;
  vu32 t1d = TIMER1_DATA;
  u32 clk = (t1d<<16) | t0d;

  return clk;
}

typedef struct {
  char *luapath;
  lua_State *L;
} GameState;

static void init_lua(GameState *state) {
  state->L = lua_open();
  luaL_openlibs(state->L);
}

GameState *init_state() {
  static GameState state;
  PA_Init();

  PA_LoadDefaultText(0, 1);
  PA_LoadDefaultText(1, 1);
  PA_OutputSimpleText(0, 0, 0, "Nintendo DS Game Demo!");

  if (!fatInitDefault()) {
    PA_OutputSimpleText(0, 0, 0, "cannot init libfat");
    exit(1);
  }

  init_lua(&state);

  return &state;
}

static void runlua(GameState *state, const char *luapath) {
  PA_OutputText(0, 0, 1, "load: %s", luapath);
  if (luaL_dofile(state->L, luapath)) {
    PA_OutputSimpleText(0, 0, 0, "cannot load lua");
    exit(1);
  }
}

static float64 testlua(GameState *state, const char *luapath) {
  u32 t;
  PrfStart();
  runlua(state, luapath);
  t = PrfEnd();
  return (float64)t / TIMER_CLOCK;
}

void mainloop(GameState *state) {
  u32 t;

  consoleDemoInit();
  printf("per %d\n", TIMER_CLOCK);

  printf("game.lua: %f\n", testlua(state, "game.lua"));

  while(1) {
    PrfStart();
    PA_WaitForVBL();
    t = PrfEnd();
    printf("\x1b[sloop: %f\x1b[u", (float64)t / TIMER_CLOCK);
  }
}

int main(int argc, char **argv) {
  GameState *state = init_state();;

  mainloop(state);

  return 0;
}
