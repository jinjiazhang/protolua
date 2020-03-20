#ifndef _JINJIAZHANG_PROTOVER_H_
#define _JINJIAZHANG_PROTOVER_H_

#include "lua.hpp"
#define lua_tolonglong lua_tointeger
#define lua_pushlonglong lua_pushinteger

#if LUA_VERSION_NUM == 501

#undef lua_tolonglong
#undef lua_pushlonglong

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

inline long long lua_tolonglong(lua_State *L, int idx)
{
    return *(long long*)lua_tostring(L, idx);
}

inline void lua_pushlonglong(lua_State *L, long long n)
{
    lua_pushlstring(L, (const char*)n, sizeof(n));
}

#endif

#endif