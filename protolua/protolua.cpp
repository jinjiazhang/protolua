#include "protolua.h"

// ret = proto.parse("preson.proto")
static int parse(lua_State *L)
{
    assert(lua_gettop(L) == 1);
    const char* file = lua_tostring(L, 1);
    if (!ProtoParse(file))
    {
        proto_error("proto.parse fail, file=%s\n", file);
        lua_pushboolean(L, false);
        return 1;
    }

    lua_pushboolean(L, true);
    return 1;
}

// person = proto.build("Person")
static int build(lua_State *L)
{
    assert(lua_gettop(L) == 1);
    const char* proto = lua_tostring(L, 1);
    if (!ProtoDecode(proto, L, 0, 0))
    {
        proto_error("proto.build fail, proto=%s\n", proto);
        return 0;
    }

    return lua_gettop(L) - 1;
}

// data = proto.encode("Person", person)
static int encode(lua_State *L)
{
    assert(lua_gettop(L) == 2);
    assert(lua_istable(L, 2));
    const char* proto = lua_tostring(L, 1);
    if (!ProtoEncode(proto, L, 2, 0, 0))
    {
        proto_error("proto.encode fail, proto=%s\n", proto);
        return 0;
    }

    return lua_gettop(L) - 2;
}

// person = proto.decode("Person", data)
static int decode(lua_State *L)
{
    assert(lua_gettop(L) == 2);
    size_t size = 0;
    const char* proto = lua_tostring(L, 1);
    const char* data = lua_tolstring(L, 2, &size);
    if (!ProtoDecode(proto, L, data, size))
    {
        proto_error("proto.decode fail, proto=%s\n", proto);
        return 0;
    }

    return lua_gettop(L) - 2;
}

// data = proto.pack("Person", name, id, email)
static int pack(lua_State *L)
{
    assert(lua_gettop(L) >= 1);
    int stack = lua_gettop(L);
    const char* proto = lua_tostring(L, 1);
    if (!ProtoPack(proto, L, 2, stack, 0, 0))
    {
        proto_error("proto.pack fail, proto=%s\n", proto);
        return 0;
    }

    return lua_gettop(L) - stack;
}

// name, id, email = proto.unpack("Person", data)
static int unpack(lua_State *L)
{
    assert(lua_gettop(L) == 2);
    size_t size = 0;
    const char* proto = lua_tostring(L, 1);
    const char* data = lua_tolstring(L, 2, &size);
    if (!ProtoUnpack(proto, L, data, size))
    {
        proto_error("proto.unpack fail, proto=%s\n", proto);
        return 0;
    }
    
    return lua_gettop(L) - 2;
}

static const struct luaL_Reg protoLib[]={
    {"parse", parse},
    {"build", build},
    {"encode", encode},
    {"decode", decode},
    {"pack", pack},
    {"unpack", unpack},
    {NULL, NULL}
};

bool ProtoParse(const char* file)
{
    const FileDescriptor* parsed_file = g_importer.Import(file);
    if (parsed_file == NULL) {
        return false;
    }
    return true;
}

void proto_log(int level, const char* format, ...)
{
    va_list	args;
    va_start(args, format);
#ifdef _WIN32
    _vprintf_p(format, args);
#else
    vprintf(format, args);
#endif // _WIN32
    va_end(args);
}

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
};

ProtoErrorCollector		g_errorCollector;
DiskSourceTree			g_sourceTree;
Importer				g_importer(&g_sourceTree, &g_errorCollector);
DynamicMessageFactory	g_factory;

#ifdef _WIN32
extern "C" __declspec(dllexport) int luaopen_protolua(lua_State* L)  
#else
extern "C" int luaopen_protolua(lua_State* L)
#endif // _WIN32
{
    lua_newtable(L);
    luaL_setfuncs(L, protoLib, 0);
    lua_setglobal(L, "proto");
    
    g_sourceTree.MapPath("", "./");
    return 1;
}