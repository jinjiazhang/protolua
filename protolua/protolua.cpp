#include "protolua.h"

using namespace google::protobuf;
using namespace google::protobuf::compiler;

void proto_init(lua_State* L);
bool proto_reload(lua_State* L);

// ret = proto.parse("preson.proto")
static int parse(lua_State *L)
{
    assert(lua_gettop(L) == 1);
    luaL_checktype(L, 1, LUA_TSTRING);
    const char* file = lua_tostring(L, 1);
    if (!proto_parse(file, L))
    {
        proto_error("proto.parse fail, file=%s", file);
        lua_pushboolean(L, false);
        return 1;
    }

    lua_pushboolean(L, true);
    return 1;
}

// is_exist = proto.exist("Person")
static int exist(lua_State *L)
{
    assert(lua_gettop(L) == 1);
    luaL_checktype(L, 1, LUA_TSTRING);
    const char* proto = lua_tostring(L, 1);
    if (!g_importer->pool()->FindMessageTypeByName(proto))
    {
        lua_pushboolean(L, 0);
    }
    else
    {
        lua_pushboolean(L, 1);
    }
    return 1;
}

// person = proto.create("Person")
static int create(lua_State *L)
{
    assert(lua_gettop(L) == 1);
    luaL_checktype(L, 1, LUA_TSTRING);
    const char* proto = lua_tostring(L, 1);
    if (!proto_create(proto, L))
    {
        proto_error("proto.create fail, proto=%s", proto);
        return 0;
    }

    return lua_gettop(L) - 1;
}

// data = proto.encode("Person", person)
static int encode(lua_State *L)
{
    assert(lua_gettop(L) == 2);
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TTABLE);
    const char* proto = lua_tostring(L, 1);
    if (!proto_encode(proto, L, 2, 0, 0))
    {
        proto_error("proto.encode fail, proto=%s", proto);
        return 0;
    }

    return lua_gettop(L) - 2;
}

// person = proto.decode("Person", data)
static int decode(lua_State *L)
{
    assert(lua_gettop(L) == 2);
    size_t size = 0;
    luaL_checktype(L, 1, LUA_TSTRING);
    const char* proto = lua_tostring(L, 1);
    luaL_checktype(L, 2, LUA_TSTRING);
    const char* data = lua_tolstring(L, 2, &size);
    if (!proto_decode(proto, L, data, size))
    {
        proto_error("proto.decode fail, proto=%s", proto);
        return 0;
    }

    return lua_gettop(L) - 2;
}

// data = proto.pack("Person", name, id, email)
static int pack(lua_State *L)
{
    assert(lua_gettop(L) >= 1);
    int stack = lua_gettop(L);
    luaL_checktype(L, 1, LUA_TSTRING);
    const char* proto = lua_tostring(L, 1);
    if (!proto_pack(proto, L, 2, stack, 0, 0))
    {
        proto_error("proto.pack fail, proto=%s", proto);
        return 0;
    }

    return lua_gettop(L) - stack;
}

// name, id, email = proto.unpack("Person", data)
static int unpack(lua_State *L)
{
    assert(lua_gettop(L) == 2);
    size_t size = 0;
    luaL_checktype(L, 1, LUA_TSTRING);
    const char* proto = lua_tostring(L, 1);
    const char* data = lua_tolstring(L, 2, &size);
    if (!proto_unpack(proto, L, data, size))
    {
        proto_error("proto.unpack fail, proto=%s", proto);
        return 0;
    }
    
    return lua_gettop(L) - 2;
}

// proto.reload()
static int reload(lua_State *L)
{
    if (!proto_reload(L))
    {
        proto_error("proto.reload fail");
        lua_pushboolean(L, false);
        return 1;
    }

    lua_pushboolean(L, true);
    return 1;
}

static const struct luaL_Reg protoLib[]={
    {"parse", parse},
    {"exist", exist},
    {"create", create},
    {"encode", encode},
    {"decode", decode},
    {"pack", pack},
    {"unpack", unpack},
    {"reload", reload},
    {NULL, NULL}
};

LUA_API int luaopen_protolua(lua_State* L)
{
    proto_init(L);

#if LUA_VERSION_NUM == 501
    luaL_register(L, "proto", protoLib);
#else
    lua_newtable(L);
    luaL_setfuncs(L, protoLib, 0);
    lua_setglobal(L, "proto");
#endif
    return 1;
}