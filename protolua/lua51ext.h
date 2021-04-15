#ifndef _JINJIAZHANG_PROTOVER_H_
#define _JINJIAZHANG_PROTOVER_H_

#include "lua.hpp"

#ifndef lua_toint64
#define lua_toint64 lua_toint64
#endif

#ifndef lua_pushint64
#define lua_pushint64 lua_pushint64
#endif

inline int lua_geti(lua_State *L, int idx, lua_Integer n)
{
    lua_pushinteger(L, n);
    lua_gettable(L, idx);
    return lua_gettop(L);
}

inline void lua_seti(lua_State *L, int idx, lua_Integer n)
{
    lua_pushinteger(L, n);
    lua_insert(L, -2);
    lua_settable(L, idx - 1);
}

inline int lua_absindex(lua_State *L, int idx)
{
    if (idx <= 0 && idx > LUA_REGISTRYINDEX)
        idx = lua_gettop(L) + idx + 1;
    return idx;
}

inline lua_Integer luaL_len(lua_State *L, int idx)
{
    return luaL_getn(L, idx);
}

inline long long lua_toint64(lua_State *L, int idx)
{
    return *(long long*)lua_tostring(L, idx);
}

inline void lua_pushint64(lua_State *L, long long n)
{
    lua_pushlstring(L, (const char*)n, sizeof(n));
}

#endif