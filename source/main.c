#include <PA9.h>
#include <fat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "dsbind.h"

typedef struct {
  char *luapath;
  lua_State *L;
} GameState;

static void init_lua(GameState *state) {
  if (state->L) {
    lua_close(state->L);
  }
  state->L = lua_open();
  luaL_openlibs(state->L);
  luaopen_dslib(state->L);
}

void init_state(GameState *state) {
  PA_Init();
  consoleDemoInit();

  iprintf("Nintendo DS Game Demo!\n");

  if (!fatInitDefault()) {
    iprintf("cannot init libfat\n");
    exit(1);
  }

  init_lua(state);
}

static void luareport(GameState *state) {
  lua_State *L = state->L;
  if (!lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    iprintf("error: %s\n", msg);
    lua_pop(L, 1);
  }
}

static int loadlua(GameState *state, const char *luapath) {
  state->luapath = (char*)luapath;
  iprintf("load: %s\n", luapath);
  return (luaL_loadfile(state->L, luapath));
}

static int runlua(GameState *state, const char *luapath) {
  return loadlua(state, luapath) || lua_pcall(state->L, 0, LUA_MULTRET, 0);
}

int main(int argc, char **argv) {
  while (1) {
    static GameState state = {NULL, NULL};
    setjmp(resetbuf);

    init_state(&state);
    if (runlua(&state, "game.lua")) {
      luareport(&state);
      while (1)
        PA_WaitForVBL();
    }
  }

  return 0;
}
