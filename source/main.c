#include <PA9.h>
#include <fat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <setjmp.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define SYSTEM_TIMER_CLOCK (2<<25)
#define TIMER_DIV TIMER_DIV_256
#define TIMER_CLOCK (SYSTEM_TIMER_CLOCK/256)

#define LUA_FUNC static

static jmp_buf mainbuf;


__attribute__((noinline)) LUA_FUNC int lua_profilestart(lua_State *L) {
  TIMER0_CR = 0;
  TIMER0_DATA = 0;
  TIMER0_CR = TIMER_ENABLE | TIMER_DIV;
  TIMER1_CR = 0;
  TIMER1_DATA = 0;
  TIMER1_CR = TIMER_ENABLE | TIMER_CASCADE;
  return 0;
}

__attribute__((noinline)) LUA_FUNC int lua_profileend(lua_State *L) {
  vu32 t0d = TIMER0_DATA;
  vu32 t1d = TIMER1_DATA;
  u32 clk = (t1d<<16) | t0d;
  printf("time: %.5fs\n", (float64)clk / TIMER_CLOCK);
  return 0;
}

inline static void draw(u8 screen, s16 x, s16 y, u16 color) {
  PA_Put16bitPixel(screen, x, y, color);
}

inline static void drawline(u8 screen, s16 x1, s16 y1, s16 x2, s16 y2, u16 color) {
  PA_Draw16bitLine(screen, x1, y1, x2, y2, color);
}

inline static void drawrect(u8 screen, s16 x1, s16 y1, s16 x2, s16 y2, u16 color) {
  PA_Draw16bitRect(screen, x1, y1, x2, y2, color);
}


LUA_FUNC int lua_draw(lua_State *L) {
  u8 screen = luaL_checkint(L, -6);
  u16 x = luaL_checkint(L, -5);
  u16 y = luaL_checkint(L, -4);
  u8 r = luaL_checkint(L, -3);
  u8 g = luaL_checkint(L, -2);
  u8 b = luaL_checkint(L, -1);

  draw(screen, x, y, PA_RGB(r, g, b));

  return 0;
}

LUA_FUNC int lua_drawline(lua_State *L) {
  u8 screen = luaL_checkint(L, -8);
  u16 x1 = luaL_checkint(L, -7);
  u16 y1 = luaL_checkint(L, -6);
  u16 x2 = luaL_checkint(L, -5);
  u16 y2 = luaL_checkint(L, -4);
  u8 r = luaL_checkint(L, -3);
  u8 g = luaL_checkint(L, -2);
  u8 b = luaL_checkint(L, -1);

  drawline(screen, x1, y1, x2, y2, PA_RGB(r, g, b));

  return 0;
}

LUA_FUNC int lua_drawrect(lua_State *L) {
  u8 screen = luaL_checkint(L, -8);
  u16 x1 = luaL_checkint(L, -7);
  u16 y1 = luaL_checkint(L, -6);
  u16 x2 = luaL_checkint(L, -5);
  u16 y2 = luaL_checkint(L, -4);
  u8 r = luaL_checkint(L, -3);
  u8 g = luaL_checkint(L, -2);
  u8 b = luaL_checkint(L, -1);

  drawrect(screen, x1, y1, x2, y2, PA_RGB(r, g, b));

  return 0;
}

#define UPDATEB(b,f) \
  lua_pushboolean(L, (b)); \
  lua_setfield(L, -2, (f))
#define UPDATEI(i,f) \
  lua_pushinteger(L, (i)); \
  lua_setfield(L, -2, (f))
static void update_pads(lua_State *L) {
#define UPDATEPAD(t, s) \
  lua_getfield(L, -1, t); \
  UPDATEB((s).A, "a"); \
  UPDATEB((s).B, "b"); \
  UPDATEB((s).X, "x"); \
  UPDATEB((s).Y, "y"); \
  UPDATEB((s).L, "l"); \
  UPDATEB((s).R, "r"); \
  UPDATEB((s).Up, "up"); \
  UPDATEB((s).Down, "down"); \
  UPDATEB((s).Right, "right"); \
  UPDATEB((s).Left, "left"); \
  UPDATEB((s).Start, "start"); \
  UPDATEB((s).Select, "select"); \
  UPDATEB((s).Anykey, "anykey"); \
  lua_pop(L, 1)

  lua_getglobal(L, "dslib");
  lua_getfield(L, -1, "pads");

  UPDATEPAD("held", Pad.Held);
  UPDATEPAD("released", Pad.Released);
  UPDATEPAD("newpress", Pad.Newpress);
}

static void update_stylus(lua_State *L) {
  lua_getglobal(L, "dslib");
  lua_getfield(L, -1, "stylus");

  UPDATEB(Stylus.Held, "held");
  UPDATEB(Stylus.Released, "released");
  UPDATEB(Stylus.Newpress, "newpress");
  UPDATEB(Stylus.Newpress0, "newpress0");

  UPDATEI(Stylus.X, "x");
  UPDATEI(Stylus.Y, "y");
  UPDATEI(Stylus.altX, "altx");
  UPDATEI(Stylus.altY, "alty");
  UPDATEI(Stylus.Pressure, "pressure");
  UPDATEI(Stylus.Vx, "vx");
  UPDATEI(Stylus.Vy, "vy");
  UPDATEI(Stylus.oldVx, "oldvx");
  UPDATEI(Stylus.oldVy, "oldvy");
  UPDATEI(Stylus.Downtime, "downtime");
  UPDATEI(Stylus.Uptime, "uptime");
  UPDATEI(Stylus.DblClick, "dblclick");
}
#undef UPDATEI
#undef UPDATEB

LUA_FUNC int lua_waitvbl(lua_State *L) {
  PA_WaitForVBL();
  update_pads(L);
  update_stylus(L);
  return 0;
}

LUA_FUNC int lua_vram_mode(lua_State *L) {
  PA_SetBgPalCol(0, 0, PA_RGB(0, 0, 0));
  PA_Init16bitBg(0, 3);
  return 0;
}

LUA_FUNC int lua_text_mode(lua_State *L) {
  PA_InitText(0, 0);
  PA_SetTextCol(0, 31, 31, 31);
  return 0;
}

LUA_FUNC int lua_textcol(lua_State *L) {
  int screen = luaL_checkint(L, -4);
  int r = luaL_checkint(L, -3);
  int g = luaL_checkint(L, -2);
  int b = luaL_checkint(L, -1);
  PA_SetTextCol(screen, r, g, b);
  return 0;
}

LUA_FUNC int lua_texttilecol(lua_State *L) {
  int screen = luaL_checkint(L, -2);
  int n = luaL_checkint(L, -1);
  PA_SetTextTileCol(screen, n);
  return 0;
}

LUA_FUNC int lua_text(lua_State *L) {
  int screen = luaL_checkint(L, -4);
  int x = luaL_checkint(L, -3);
  int y = luaL_checkint(L, -2);
  const char *text = luaL_checklstring(L, -1, NULL);
  u16 letters = PA_OutputSimpleText(screen, x, y, text);
  lua_pushinteger(L, letters);
  return 1;
}

LUA_FUNC int lua_cleartext(lua_State *L) {
  s8 screen = luaL_checkint(L, -1);
  PA_ClearTextBg(screen);
  return 0;
}

LUA_FUNC int lua_ls(lua_State *L) {
  DIR *cdir;
  struct dirent *ent;
  struct stat statbuf;
  s16 filecount = 0;

  lua_text_mode(L);

  cdir = opendir(".");
  if (!cdir) {
    iprintf("cannot open ./\n");
    exit(1);
  }

  while ((ent = readdir(cdir)) != NULL) {
    char *dname = ent->d_name;
    stat(dname, &statbuf);
    if (S_ISDIR(statbuf.st_mode)) continue;
    lua_pushstring(L, dname);
    ++filecount;
  }
  closedir(cdir);

  return filecount;
}

static const luaL_Reg ds_funcs[] = {
  {"profile_start", lua_profilestart},
  {"profile_end", lua_profileend},
  {"draw", lua_draw},
  {"drawline", lua_drawline},
  {"drawrect", lua_drawrect},
  {"wait", lua_waitvbl},
  {"ls", lua_ls},
  {"vram_mode", lua_vram_mode},
  {"text_mode", lua_text_mode},
  {"text", lua_text},
  {"textcol", lua_textcol},
  {"texttilecol", lua_texttilecol},
  {"cleartext", lua_cleartext},
  {NULL, NULL}
};

typedef struct {
  char *luapath;
  lua_State *L;
} GameState;

static int loadlua(GameState *state, const char *luapath) {
  state->luapath = (char*)luapath;
  iprintf("load: %s\n", luapath);
  return (luaL_loadfile(state->L, luapath));
}

static int runlua(GameState *state) {
  return lua_pcall(state->L, 0, LUA_MULTRET, 0);
}

static int testlua(GameState *state, const char *luapath) {
  return loadlua(state, luapath) || runlua(state);
}

static void init_dsfuncs(lua_State *L) {
  luaL_register(L, "dslib", ds_funcs);
  lua_getglobal(L, "dslib");

  lua_createtable(L, 0, 3);
  lua_createtable(L, 0, 13);
  lua_setfield(L, -2, "held");
  lua_createtable(L, 0, 13);
  lua_setfield(L, -2, "released");
  lua_createtable(L, 0, 13);
  lua_setfield(L, -2, "newpress");
  lua_setfield(L, -2, "pads");

  lua_createtable(L, 0, 16);
  lua_setfield(L, -2, "stylus");

  lua_pop(L, 1);
}

static void init_lua(GameState *state) {
  if (state->L) {
    lua_close(state->L);
  }
  state->L = lua_open();
  luaL_openlibs(state->L);
  init_dsfuncs(state->L);
}

GameState *init_state() {
  static GameState state = {NULL, NULL};

  PA_Init();
  consoleDemoInit();

  iprintf("Nintendo DS Game Demo!\n");

  if (!fatInitDefault()) {
    iprintf("cannot init libfat\n");
    exit(1);
  }

  init_lua(&state);

  return &state;
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

void statereset() {
  longjmp(mainbuf, 1);
}

int main(int argc, char **argv) {
  while (1) {
    GameState *state;
    setjmp(mainbuf);

    state = init_state();
    if (testlua(state, "game.lua")) {
      luareport(state);
      while (1)
        PA_WaitForVBL();
    }
  }

  return 0;
}
