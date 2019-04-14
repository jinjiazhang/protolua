#include "protolua.h"

using namespace google::protobuf;
using namespace google::protobuf::compiler;

bool parse_enum(const EnumDescriptor* enum_desc, lua_State* L)
{
    lua_getglobal(L, enum_desc->name().c_str());
    PROTO_ASSERT(lua_isnil(L, -1));
    lua_pop(L, 1);

    lua_newtable(L);
    int value_count = enum_desc->value_count();
    for (int i = 0; i < value_count; i++)
    {
        const EnumValueDescriptor* value_desc = enum_desc->value(i);
        lua_pushstring(L, value_desc->name().c_str());
        lua_pushinteger(L, value_desc->number());
        lua_settable(L, -3);
    }
    lua_setglobal(L, enum_desc->name().c_str());
    return true;
}

bool parse_message(const Descriptor* message_desc, lua_State* L)
{
    int emum_count = message_desc->enum_type_count();
    for (int i = 0; i < emum_count; i++)
    {
        const EnumDescriptor* enum_desc = message_desc->enum_type(i);
        PROTO_DO(parse_enum(enum_desc, L));
    }

    int nest_count = message_desc->nested_type_count();
    for (int i = 0; i < nest_count; i++)
    {
        const Descriptor* nest_desc = message_desc->nested_type(i);
        PROTO_DO(parse_message(nest_desc, L));
    }
    return true;
}

bool parse_file(const FileDescriptor* file_desc, lua_State* L)
{
    int dep_count = file_desc->dependency_count();
    for (int i = 0; i < dep_count; i++)
    {
        const FileDescriptor* dep_file = file_desc->dependency(i);
        PROTO_DO(parse_file(dep_file, L));
    }

    int emum_count = file_desc->enum_type_count();
    for (int i = 0; i < emum_count; i++)
    {
        const EnumDescriptor* enum_desc = file_desc->enum_type(i);
        PROTO_DO(parse_enum(enum_desc, L));
    }

    int message_count = file_desc->message_type_count();
    for (int i = 0; i < message_count; i++)
    {
        const Descriptor* message_desc = file_desc->message_type(i);
        PROTO_DO(parse_message(message_desc, L));
    }
    return true;
}

bool proto_parse(const char* file, lua_State* L)
{
    const FileDescriptor* parsed_file = g_importer.Import(file);
    if (parsed_file == NULL) {
        return false;
    }

    PROTO_DO(parse_file(parsed_file, L));
    return true;
}