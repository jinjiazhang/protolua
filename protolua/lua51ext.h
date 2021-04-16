#ifndef _JINJIAZHANG_PROTOVER_H_
#define _JINJIAZHANG_PROTOVER_H_

#include <stdlib.h>
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

inline void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup) {
    luaL_checkstack(L, nup, "too many upvalues");
    for (; l->name != NULL; l++) {  /* fill the table with given functions */
        int i;
        for (i = 0; i < nup; i++)  /* copy upvalues to the top */
            lua_pushvalue(L, -nup);
        lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
        lua_setfield(L, -(nup + 2), l->name);
    }
    lua_pop(L, nup);  /* remove upvalues */
}

inline long long lua_toint64(lua_State *L, int idx)
{
    if (lua_type(L, idx) == LUA_TSTRING)
    {
        return atoll(lua_tostring(L, idx));
    }
    return (long long)lua_tonumber(L, idx);
}

inline void lua_pushint64(lua_State *L, long long value)
{
    // because of #define LUA_NUMBER_FMT "%.14g"
    if (abs(value) > 99999999999999)
    {
        char str[32];
        lua_pushstring(L, _i64toa(value, str, 10));
        return;
    }

    lua_pushnumber(L, (lua_Number)value);
}

#endif