#include <PA9.h>
#include "all_gfx.h"
#include "fat.h"

#include "lua-5.1.4/src/lua.h"
#include "lua-5.1.4/src/lauxlib.h"
#include "lua-5.1.4/src/lualib.h"


#define N_SPRITES 1

typedef struct {
  s32 vx, vy;
} SpriteState;

typedef struct {
  u16 nsprites;
  SpriteState sprites[PA_NMAXSPRITES];
  char *confpath;
  lua_State *L;
} GameState;

u16 add_sprite(GameState *state, u16 gfx) {
  u16 i = state->nsprites;
  s32 w = 64;
  s32 h = 64;
  s32 x = pa_3dsprites[i].X = 32+(PA_Rand()%224);
  s32 y = pa_3dsprites[i].Y = 32+(PA_Rand()%160);
  PA_3DCreateSpriteFromTex(state->nsprites, gfx, w, h, 0, x, y);
  //PA_3DSetSpriteWidthHeight(i, 32, 32); // Resize
  return ++state->nsprites;
}

void init_sprites(GameState *state) {
  s32 i;
  u16 gfx = PA_3DCreateTex((void*)untan_Texture, 64, 64, TEX_16BITS);

  for(i = 0; i < N_SPRITES; i++) {
    u16 angle = PA_Rand()&511; // random angle
    add_sprite(state, gfx);
    state->sprites[i].vx = PA_Cos(angle)>>7;
    state->sprites[i].vy = -PA_Sin(angle)>>7;
  }
}

static void init_lua(GameState *state) {
  state->L = lua_open();
  luaL_openlibs(state->L);
}

static s16 loadconf(GameState *state) {
  if (luaL_dofile(state->L, state->confpath)) {
    PA_BoxText(0, 0, 0, 100, 1, "cannot load lua", 100);
    return 1;
  }
  PA_BoxText(1, 0, 10, 4, 1, "load", 100);
  PA_BoxText(1, 6, 10, 100, 1, state->confpath, 100);
  return 0;
}

GameState *init_state(char *confpath) {
  static GameState state;
  PA_Init();
  PA_Init3D(); // Uses Bg0
  PA_Reset3DSprites();

  // Initialise the text system on the top screen
  PA_LoadDefaultText(0, 1);
  PA_LoadDefaultText(1, 1);
  //PA_OutputSimpleText(1, 2, 6, "Nintendo DS Game Demo!");

  state.nsprites = 0;

  init_sprites(&state);

  if (!fatInitDefault()) {
    PA_BoxText(0, 0, 0, 100, 1, "cannot init libfat", 100);
    exit(1);
  }

  state.confpath = confpath;
  init_lua(&state);

  return &state;
}

static inline void PA_3DMoveSpriteXY(u16 sprite, s16 x, s16 y) {
  pa_3dsprites[sprite].X += x;
  pa_3dsprites[sprite].Y += y;
}

void mainloop(GameState *state) {
  s32 i;
  SpriteState *sprites = state->sprites;
  loadconf(state);
  while(1) {
    for(i = 0; i < N_SPRITES; i++){
      // Move them around and change speed if touches screen
      PA_3DMoveSpriteXY(i, sprites[i].vx, sprites[i].vy);
      if (((pa_3dsprites[i].X <= 32)&&(sprites[i].vx < 0))||((pa_3dsprites[i].X>=255-32)&&(sprites[i].vx > 0)))
        sprites[i].vx = -sprites[i].vx;
      if (((pa_3dsprites[i].Y <= 32)&&(sprites[i].vy < 0))||((pa_3dsprites[i].Y>=191-32)&&(sprites[i].vy > 0)))
        sprites[i].vy = -sprites[i].vy;
    }

    PA_WaitForVBL();
    PA_3DProcess();
  }
}

int main(int argc, char **argv) {
  GameState *state = init_state("game.lua");;

  mainloop(state);

  return 0;
}
