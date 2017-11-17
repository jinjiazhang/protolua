#include "proto.h"

char cache_buffer[2*1024];
DescriptorPool* g_descriptor_pool = NULL;
MessageFactory* g_message_factory = NULL;

// ret = proto.parse("preson.proto")
static int parse(lua_State *L)
{
    assert(lua_gettop(L) == 1);
    const char* file = lua_tostring(L, 1);
    if (!ProtoParse(file))
    {
        ProtoError("proto.parse fail, file=%s\n", file);
        lua_pushboolean(L, false);
        return 1;
    }

    lua_pushboolean(L, true);
    return 1;
}

// data = proto.encode("Person", person)
static int encode(lua_State *L)
{
    assert(lua_gettop(L) == 2);
    assert(lua_istable(L, 2));
    const char* proto = lua_tostring(L, 1);
    size_t size = sizeof(cache_buffer);
    if (!ProtoEncode(proto, L, 2, cache_buffer, &size))
    {
        ProtoError("proto.encode fail, proto=%s\n", proto);
        return 0;
    }

    lua_pushlstring(L, cache_buffer, size);
    return 1;
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
        ProtoError("proto.decode fail, proto=%s\n", proto);
        return 0;
    }

    int stack = lua_gettop(L);
    return stack - 2;
}

// data = proto.pack("Person", name, id, email)
static int pack(lua_State *L)
{
    assert(lua_gettop(L) >= 1);
    int stack = lua_gettop(L);
    const char* proto = lua_tostring(L, 1);
    size_t size = sizeof(cache_buffer);
    if (!ProtoPack(proto, L, 2, stack, cache_buffer, &size))
    {
        ProtoError("proto.pack fail, proto=%s\n", proto);
        return 0;
    }

    lua_pushlstring(L, cache_buffer, size);
    return 1;
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
        ProtoError("proto.unpack fail, proto=%s\n", proto);
        return 0;
    }
    
    int stack = lua_gettop(L);
    return stack - 2;
}

static const struct luaL_Reg protoLib[]={
    {"parse", parse},
    {"encode", encode},
    {"decode", decode},
    {"pack", pack},
    {"unpack", unpack},
    {NULL, NULL}
};

bool ProtoParse(const char* file)
{
    g_descriptor_pool->AddUnusedImportTrackFile(file);
    const FileDescriptor* parsed_file = g_descriptor_pool->FindFileByName(file);
    g_descriptor_pool->ClearUnusedImportTrackFiles();
    if (parsed_file == NULL) {
        return false;
    }
    return true;
}

bool ProtoError(const char* format, ...)
{
    va_list	args;
    va_start(args, format);
    _vprintf_p(format, args);
    va_end(args);
    return true;
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

#ifdef _WIN32  
extern "C" __declspec(dllexport) int luaopen_protolua(lua_State* L)  
#else
extern "C" int luaopen_protolua(lua_State* L)
#endif // _WIN32
{
    lua_newtable(L);
    luaL_setfuncs(L, protoLib, 0);
    lua_setglobal(L, "proto");
    
    memset(cache_buffer, 0, sizeof(cache_buffer));
    DiskSourceTree* disk_source_tree = new DiskSourceTree();
    disk_source_tree->MapPath("", "./");
    DescriptorDatabase* descriptor_database = new SourceTreeDescriptorDatabase(disk_source_tree);
    g_descriptor_pool = new DescriptorPool(descriptor_database);
    g_descriptor_pool->EnforceWeakDependencies(true);
    g_message_factory = new DynamicMessageFactory(g_descriptor_pool);
    return 1;
}