
#define LUA_FUNC static

#define DSLIBNAME "dslib"

extern jmp_buf resetbuf;

LUALIB_API int luaopen_dslib(lua_State *L);
