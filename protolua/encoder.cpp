#include "protolua.h"

using namespace google::protobuf;
using namespace google::protobuf::compiler;

bool encode_field(Message* message, const FieldDescriptor* field, lua_State* L, int index);
bool encode_required(Message* message, const FieldDescriptor* field, lua_State* L, int index);
bool encode_optional(Message* message, const FieldDescriptor* field, lua_State* L, int index);
bool encode_repeated(Message* message, const FieldDescriptor* field, lua_State* L, int index);
bool encode_table(Message* message, const FieldDescriptor* field, lua_State* L, int index);
bool encode_single(Message* message, const FieldDescriptor* field, lua_State* L, int index);
bool encode_multiple(Message* message, const FieldDescriptor* field, lua_State* L, int index);
bool encode_message(Message* message, const Descriptor* descriptor, lua_State* L, int index);

bool encode_field(Message* message, const FieldDescriptor* field, lua_State* L, int index)
{
    if (field->is_map())
        return encode_table(message, field, L, index);
    else if (field->is_required())
        return encode_required(message, field, L, index);
    else if (field->is_optional())
        return encode_optional(message, field, L, index);
    else if (field->is_repeated())
        return encode_repeated(message, field, L, index);
    else
        return false;
}

bool encode_required(Message* message, const FieldDescriptor* field, lua_State* L, int index)
{
    if (lua_isnil(L, index)) {
        proto_warn("encode_required field nil, field=%s", field->full_name().c_str());
        return true;
    }

    return encode_single(message, field, L, index);
}

bool encode_optional(Message* message, const FieldDescriptor* field, lua_State* L, int index)
{
    if (lua_isnil(L, index)) {
        return true;
    }

    return encode_single(message, field, L, index);
}

bool encode_repeated(Message* message, const FieldDescriptor* field, lua_State* L, int index)
{
    if (lua_isnil(L, index)) {
        return true;
    }

    if (!lua_istable(L, index)) {
        proto_error("encode_repeated field isn't a table, field=%s", field->full_name().c_str());
        return false;
    }

    int count = (int)luaL_len(L, index);
    for (int i = 0; i < count; i++)
    {
        lua_geti(L, index, i + 1);
        PROTO_DO(encode_multiple(message, field, L, lua_absindex(L, -1)));
        lua_pop(L, 1);
    }
    return true;
}

bool encode_table(Message* message, const FieldDescriptor* field, lua_State* L, int index)
{
    if (lua_isnil(L, index)) {
        return true;
    }

    if (!lua_istable(L, index)) {
        proto_error("encode_table field isn't a table, field=%s", field->full_name().c_str());
        return false;
    }

    const Reflection* reflection = message->GetReflection();
    const Descriptor* descriptor = field->message_type();
    PROTO_ASSERT(descriptor->field_count() == 2);
    const FieldDescriptor* key = descriptor->field(0);
    const FieldDescriptor* value = descriptor->field(1);

    lua_pushnil(L);
    while (lua_next(L, index))
    {
        Message* submessage = reflection->AddMessage(message, field);
        PROTO_DO(encode_field(submessage, key, L, lua_absindex(L, -2)));
        PROTO_DO(encode_field(submessage, value, L, lua_absindex(L, -1)));
        lua_pop(L, 1);
    }
    return true;
}

bool encode_single(Message* message, const FieldDescriptor* field, lua_State* L, int index)
{
    const Reflection* reflection = message->GetReflection();
    switch (field->cpp_type())
    {
    case FieldDescriptor::CPPTYPE_DOUBLE:
        reflection->SetDouble(message, field, (double)lua_tonumber(L, index));
        break;
    case FieldDescriptor::CPPTYPE_FLOAT:
        reflection->SetFloat(message, field, (float)lua_tonumber(L, index));
        break;
    case FieldDescriptor::CPPTYPE_INT32:
        reflection->SetInt32(message, field, (int32)lua_tointeger(L, index));
        break;
    case FieldDescriptor::CPPTYPE_UINT32:
        reflection->SetUInt32(message, field, (uint32)lua_tointeger(L, index));
        break;
    case FieldDescriptor::CPPTYPE_INT64:
        reflection->SetInt64(message, field, (int64)lua_toint64(L, index));
        break;
    case FieldDescriptor::CPPTYPE_UINT64:
        reflection->SetUInt64(message, field, (uint64)lua_toint64(L, index));
        break;
    case FieldDescriptor::CPPTYPE_ENUM:
        reflection->SetEnumValue(message, field, (int)lua_tointeger(L, index));
        break;
    case FieldDescriptor::CPPTYPE_BOOL:
        reflection->SetBool(message, field, lua_toboolean(L, index) != 0);
        break;
    case FieldDescriptor::CPPTYPE_STRING:
        {
            size_t length = 0;
            const char* bytes = lua_tolstring(L, index, &length);
            reflection->SetString(message, field, string(bytes, length));
        }
        break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
        {
            Message* submessage = reflection->MutableMessage(message, field);
            PROTO_DO(encode_message(submessage, field->message_type(), L, index));
        }
        break;
    default:
        proto_error("encode_single field unknow type, field=%s", field->full_name().c_str());
        return false;
    }
    return true;
}

bool encode_multiple(Message* message, const FieldDescriptor* field, lua_State* L, int index)
{
    const Reflection* reflection = message->GetReflection();
    switch (field->cpp_type())
    {
    case FieldDescriptor::CPPTYPE_DOUBLE:
        reflection->AddDouble(message, field, (double)lua_tonumber(L, index));
        break;
    case FieldDescriptor::CPPTYPE_FLOAT:
        reflection->AddFloat(message, field, (float)lua_tonumber(L, index));
        break;
    case FieldDescriptor::CPPTYPE_INT32:
        reflection->AddInt32(message, field, (int32)lua_tointeger(L, index));
        break;
    case FieldDescriptor::CPPTYPE_UINT32:
        reflection->AddUInt32(message, field, (uint32)lua_tointeger(L, index));
        break;
    case FieldDescriptor::CPPTYPE_INT64:
        reflection->AddInt64(message, field, (int64)lua_toint64(L, index));
        break;
    case FieldDescriptor::CPPTYPE_UINT64:
        reflection->AddUInt64(message, field, (uint64)lua_toint64(L, index));
        break;
    case FieldDescriptor::CPPTYPE_ENUM:
        reflection->AddEnumValue(message, field, (int)lua_tointeger(L, index));
        break;
    case FieldDescriptor::CPPTYPE_BOOL:
        reflection->AddBool(message, field, lua_toboolean(L, index) != 0);
        break;
    case FieldDescriptor::CPPTYPE_STRING:
        {
            size_t length = 0;
            const char* bytes = lua_tolstring(L, index, &length);
            reflection->AddString(message, field, string(bytes, length));
        }
        break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
        {
            Message* submessage = reflection->AddMessage(message, field);
            PROTO_DO(encode_message(submessage, field->message_type(), L, index));
        }
        break;
    default:
        proto_error("encode_multiple field unknow type, field=%s", field->full_name().c_str());
        return false;
    }
    return true;
}

bool encode_message(Message* message, const Descriptor* descriptor, lua_State* L, int index)
{
    if (!lua_istable(L, index)) {
        proto_error("encode_message field isn't a table, field=%s", descriptor->full_name().c_str());
        return false;
    }

    for (int i = 0; i < descriptor->field_count(); i++)
    {
        const FieldDescriptor* field = descriptor->field(i);
        lua_getfield(L, index, field->name().c_str());
        PROTO_DO(encode_field(message, field, L, lua_absindex(L, -1)));
        lua_pop(L, 1);
    }
    return true;
}

bool proto_encode(const char* proto, lua_State* L, int index, char* output, size_t* size)
{
    const Descriptor* descriptor = g_importer->pool()->FindMessageTypeByName(proto);
    PROTO_ASSERT(descriptor);

    const Message* prototype = g_factory->GetPrototype(descriptor);
    PROTO_ASSERT(prototype);

    index = lua_absindex(L, index);
    std::unique_ptr<Message> message(prototype->New());
    PROTO_DO(encode_message(message.get(), descriptor, L, index));

    if (output && size) // export to buffer
    {
        PROTO_DO(message->SerializeToArray(output, *size));
        *size = message->ByteSizeLong();
    }
    else 
    {
        string output; // push to lua stack
        PROTO_DO(message->SerializeToString(&output));
        lua_pushlstring(L, output.c_str(), output.size());
    }
    return true;
}

std::vector<const FieldDescriptor*> SortFieldsByNumber(const Descriptor* descriptor);
bool proto_pack(const char* proto, lua_State* L, int start, int end, char* output, size_t* size)
{
    const Descriptor* descriptor = g_importer->pool()->FindMessageTypeByName(proto);
    PROTO_ASSERT(descriptor);

    const Message* prototype = g_factory->GetPrototype(descriptor);
    PROTO_ASSERT(prototype);

    start = lua_absindex(L, start);
    end = lua_absindex(L, end);
    std::unique_ptr<Message> message(prototype->New());
    std::vector<const FieldDescriptor*> fields = SortFieldsByNumber(descriptor);
    for (int i = 0; i < (int)fields.size() && start + i <= end; i++)
    {
        const FieldDescriptor* field = fields[i];
        PROTO_DO(encode_field(message.get(), field, L, start + i));
    }

    if (output && size) // export to buffer
    {
        PROTO_DO(message->SerializeToArray(output, *size));
        *size = message->ByteSizeLong();
    }
    else 
    {
        string output; // push to lua stack
        PROTO_DO(message->SerializeToString(&output));
        lua_pushlstring(L, output.c_str(), output.size());
    }
    return true;
}