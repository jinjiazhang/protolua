#include "proto.h"
#include <ostream>
#include "../libs/person.pb.h"

char cache_buffer[2*1024];
DescriptorPool* g_descriptor_pool = NULL;

// ret = proto.parse("preson.proto")
static int parse(lua_State *L)
{
    assert(lua_gettop(L) == 1);
    const char* file = lua_tostring(L, 1);
    lua_pushboolean(L, ProtoParse(file));
    return 1;
}

// data = proto.encode("Person", person)
static int encode(lua_State *L)
{
    assert(lua_gettop(L) == 2);
    assert(lua_istable(L, 2));
    const char* proto = lua_tostring(L, 1);
    size_t size = sizeof(cache_buffer);
    if (ProtoEncode(proto, L, 2, cache_buffer, &size))
    {
        lua_pushlstring(L, cache_buffer, size);
        return 1;
    }
    return 0;
}

// person = proto.decode("Person", data)
static int decode(lua_State *L)
{
    assert(lua_gettop(L) == 2);
    size_t size = 0;
    const char* proto = lua_tostring(L, 1);
    const char* data = lua_tolstring(L, 2, &size);
    return ProtoDecode(proto, L, data, size);
}

// data = proto.pack("Person", name, id, email)
static int pack(lua_State *L)
{
    assert(lua_gettop(L) >= 2);
    int stack = lua_gettop(L);
    const char* proto = lua_tostring(L, 1);
    size_t size = sizeof(cache_buffer);
    if (ProtoPack(proto, L, 2, stack, cache_buffer, &size))
    {
        lua_pushlstring(L, cache_buffer, size);
        return 1;
    }
    return 0;
}

// name, id, email = proto.unpack("Person", data)
static int unpack(lua_State *L)
{
    assert(lua_gettop(L) == 2);
    size_t size = 0;
    const char* proto = lua_tostring(L, 1);
    const char* data = lua_tolstring(L, 2, &size);
    return ProtoUnpack(proto, L, data, size);
}

// proto.debug("Person", data)
static int debug(lua_State *L)
{
    assert(lua_gettop(L) == 2);
    size_t size = 0;
    const char* proto = lua_tostring(L, 1); 
    const char* data = lua_tolstring(L, 2, &size);

    string output;
    Person person;
    person.ParseFromArray((void*)data, size);
    google::protobuf::util::JsonOptions options;
    google::protobuf::util::MessageToJsonString(person, &output);
    std::cout << output.c_str() << std::endl;
    ::google::protobuf::Map< ::google::protobuf::int32, ::std::string >* dict = person.mutable_dict();
    dict->clear();
    (*dict)[1] = "abc";
    (*dict)[2] = "cba";
    memset(cache_buffer, 0, sizeof(cache_buffer));
    person.SerializeToArray(cache_buffer, sizeof(cache_buffer));
    return 0;
}

static const struct luaL_Reg protoLib[]={
    {"parse", parse},
    {"encode", encode},
    {"decode", decode},
    {"pack", pack},
    {"unpack", unpack},
    {"debug", debug},
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

void ProtoPrint(int level, const char* format, ...)
{

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
    return 1;
}