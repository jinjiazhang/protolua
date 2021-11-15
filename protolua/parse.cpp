#include "protolua.h"
#include <list>

using namespace google::protobuf;
using namespace google::protobuf::compiler;

class ProtoErrorCollector;
std::set<std::string> g_parsedFiles;
std::set<std::string> g_definedEnums;
DiskSourceTree* g_sourceTree = 0;
ProtoErrorCollector* g_errorCollector = 0;
Importer* g_importer = 0;
DynamicMessageFactory* g_factory = 0;

bool define_enum(const EnumDescriptor* enum_desc, lua_State* L)
{
    lua_getglobal(L, enum_desc->name().c_str());
    if (!lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        return true;
    }

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
    g_definedEnums.insert(enum_desc->name().c_str());
    return true;
}

bool traverse_message(const Descriptor* message_desc, lua_State* L)
{
    int emum_count = message_desc->enum_type_count();
    for (int i = 0; i < emum_count; i++)
    {
        const EnumDescriptor* enum_desc = message_desc->enum_type(i);
        PROTO_DO(define_enum(enum_desc, L));
    }

    int nest_count = message_desc->nested_type_count();
    for (int i = 0; i < nest_count; i++)
    {
        const Descriptor* nest_desc = message_desc->nested_type(i);
        PROTO_DO(traverse_message(nest_desc, L));
    }
    return true;
}

bool traverse_file(const FileDescriptor* file_desc, lua_State* L)
{
    int dep_count = file_desc->dependency_count();
    for (int i = 0; i < dep_count; i++)
    {
        const FileDescriptor* dep_file = file_desc->dependency(i);
        PROTO_DO(traverse_file(dep_file, L));
    }

    int emum_count = file_desc->enum_type_count();
    for (int i = 0; i < emum_count; i++)
    {
        const EnumDescriptor* enum_desc = file_desc->enum_type(i);
        PROTO_DO(define_enum(enum_desc, L));
    }

    int message_count = file_desc->message_type_count();
    for (int i = 0; i < message_count; i++)
    {
        const Descriptor* message_desc = file_desc->message_type(i);
        PROTO_DO(traverse_message(message_desc, L));
    }
    return true;
}

bool proto_parse(const char* file, lua_State* L)
{
    const FileDescriptor* parsed_file = g_importer->Import(file);
    if (parsed_file == NULL) {
        return false;
    }

    PROTO_DO(traverse_file(parsed_file, L));
    g_parsedFiles.insert(file);
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

void proto_init(lua_State* L)
{
    g_errorCollector = new ProtoErrorCollector();
    g_sourceTree = new DiskSourceTree();
    g_sourceTree->MapPath("", "./");
    g_sourceTree->MapPath("", "./proto/");
    g_importer = new Importer(g_sourceTree, g_errorCollector);
    g_factory = new DynamicMessageFactory();
}

bool proto_reload(lua_State* L)
{
    std::list<const FileDescriptor*> fileDescriptorList;
    Importer* importer = new Importer(g_sourceTree, g_errorCollector);
    std::set<std::string>::iterator it = g_parsedFiles.begin();
    for (; it != g_parsedFiles.end(); ++it)
    {
        const FileDescriptor* parsed_file = importer->Import(it->c_str());
        if (parsed_file == NULL)
        {
            delete importer;
            return false;
        }
        fileDescriptorList.push_back(parsed_file);
    }

    delete g_importer;
    g_importer = importer;
    
    std::set<std::string>::iterator it1 = g_definedEnums.begin();
    for (; it1 != g_definedEnums.end(); ++it1)
    {
        lua_pushnil(L);
        lua_setglobal(L, it1->c_str());
    }

    std::list<const FileDescriptor*>::iterator it2 = fileDescriptorList.begin();
    for (; it2 != fileDescriptorList.end(); ++it2)
    {
        traverse_file(*it2, L);
    }
    return true;
}
