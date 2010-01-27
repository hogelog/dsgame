#include <PA9.h>
#include <fat.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define SYSTEM_TIMER_CLOCK (2<<25)
#define TIMER_DIV TIMER_DIV_256
#define TIMER_CLOCK (SYSTEM_TIMER_CLOCK/256)

__attribute__((noinline)) static int lua_prfstart(lua_State *L) {
  TIMER0_CR = 0;
  TIMER0_DATA = 0;
  TIMER0_CR = TIMER_ENABLE | TIMER_DIV;
  TIMER1_CR = 0;
  TIMER1_DATA = 0;
  TIMER1_CR = TIMER_ENABLE | TIMER_CASCADE;
  return 0;
}

__attribute__((noinline)) static int lua_prfend(lua_State *L) {
  vu32 t0d = TIMER0_DATA;
  vu32 t1d = TIMER1_DATA;
  u32 clk = (t1d<<16) | t0d;
  printf("time: %.5fs\n", (float64)clk / TIMER_CLOCK);
  return 0;
}

inline static void draw(int x, int y, int r, int g, int b) {
  PA_Put16bitPixel(0, x, y, PA_RGB(r, g, b));
}

__attribute__((noinline)) static int lua_draw(lua_State *L) {
  int x = luaL_checkint(L, -5);
  int y = luaL_checkint(L, -4);
  int r = luaL_checkint(L, -3);
  int g = luaL_checkint(L, -2);
  int b = luaL_checkint(L, -1);

  draw(x, y, r, g, b);

  return 0;
}

typedef struct {
  char *luapath;
  lua_State *L;
} GameState;

static void init_lua(GameState *state) {
  state->L = lua_open();
  luaL_openlibs(state->L);
  lua_register(state->L, "profile_start", lua_prfstart);
  lua_register(state->L, "profile_end", lua_prfend);
  lua_register(state->L, "draw", lua_draw);
}

GameState *init_state() {
  static GameState state;
  PA_Init();
  consoleDemoInit();

  PA_SetBgPalCol(0, 0, PA_RGB(0, 0, 0));
  PA_Init16bitBg(0, 3);
  iprintf("Nintendo DS Game Demo!\n");

  if (!fatInitDefault()) {
    iprintf("cannot init libfat\n");
    exit(1);
  }

  init_lua(&state);

  return &state;
}

static void loadlua(GameState *state, const char *luapath) {
  iprintf("load: %s\n", luapath);
  if (luaL_loadfile(state->L, luapath)) {
    iprintf("cannot load lua\n");
    exit(1);
  }
}

static s32 runlua(GameState *state) {
  return lua_pcall(state->L, 0, LUA_MULTRET, 0);
}

static void testlua(GameState *state, const char *luapath) {
  loadlua(state, luapath);
  runlua(state);
}

void mainloop(GameState *state) {
  testlua(state, "game.lua");

  //s16 b = 0;
  //s16 c = 0;
  //while(1) {
  //  s16 i;
  //  iprintf("c(%d,%d,%d)\n", c, c, c);
  //  for (i=b;i<b+128;++i) {
  //    s16 j;
  //    for (j=b;j<b+128;++j) {
  //      draw(i, j, c, c, c);
  //    }
  //  }
  //  //PA_WaitForVBL();
  //  if (++b >= SCREEN_HEIGHT) b = 0;
  //  if (++c >= 32) c=0;
  //}
}

int main(int argc, char **argv) {
  GameState *state = init_state();;

  mainloop(state);

  return 0;
}
