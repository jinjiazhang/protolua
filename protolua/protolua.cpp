#include "protolua.h"

using namespace google::protobuf;
using namespace google::protobuf::compiler;

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
    if (!g_importer.pool()->FindMessageTypeByName(proto))
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
    if (!proto_decode(proto, L, 0, 0))
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

static const struct luaL_Reg protoLib[]={
    {"parse", parse},
    {"exist", exist},
    {"create", create},
    {"encode", encode},
    {"decode", decode},
    {"pack", pack},
    {"unpack", unpack},
    {NULL, NULL}
};

struct FieldOrderingByNumber {
    inline bool operator()(const FieldDescriptor* a,
        const FieldDescriptor* b) const {
            return a->number() < b->number();
    }
};

std::vector<const FieldDescriptor*> SortFieldsByNumber(const Descriptor* descriptor) {
    std::vector<const FieldDescriptor*> fields(descriptor->field_count());
    for (int i = 0; i < descriptor->field_count(); i++) {
        fields[i] = descriptor->field(i);
    }
    std::sort(fields.begin(), fields.end(), FieldOrderingByNumber());
    return fields;
}

class ProtoErrorCollector : public MultiFileErrorCollector
{
    virtual void AddError(const std::string& filename, int line, int column, const std::string& message)
    {
        proto_error("[file]%s line %d, column %d : %s", filename.c_str(), line, column, message.c_str());
    }

    virtual void AddWarning(const std::string& filename, int line, int column, const std::string& message)
    {
        proto_warn("[file]%s line %d, column %d : %s", filename.c_str(), line, column, message.c_str());
    }
};

ProtoErrorCollector        g_errorCollector;
DiskSourceTree             g_sourceTree;
Importer                   g_importer(&g_sourceTree, &g_errorCollector);
DynamicMessageFactory      g_factory;

#ifdef _WIN32
extern "C" __declspec(dllexport) int luaopen_protolua(lua_State* L)
#else
extern "C" int luaopen_protolua(lua_State* L)
#endif // _WIN32
{
#if LUA_VERSION_NUM == 501
    luaL_register(L, "proto", protoLib);
#else
    lua_newtable(L);
    luaL_setfuncs(L, protoLib, 0);
    lua_setglobal(L, "proto");
#endif
    
    g_sourceTree.MapPath("", "./");
    g_sourceTree.MapPath("", "./proto/");
    return 1;
}