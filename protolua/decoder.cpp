#include "protolua.h"

using namespace google::protobuf;
using namespace google::protobuf::compiler;

bool decode_field(const Message& message, const FieldDescriptor* field, lua_State* L);
bool decode_required(const Message& message, const FieldDescriptor* field, lua_State* L);
bool decode_optional(const Message& message, const FieldDescriptor* field, lua_State* L);
bool decode_repeated(const Message& message, const FieldDescriptor* field, lua_State* L);
bool decode_table(const Message& message, const FieldDescriptor* field, lua_State* L);
bool decode_single(const Message& message, const FieldDescriptor* field, lua_State* L);
bool decode_multiple(const Message& message, const FieldDescriptor* field, lua_State* L, int index);
bool decode_message(const Message& message, const Descriptor* descriptor, lua_State* L);

bool decode_field(const Message& message, const FieldDescriptor* field, lua_State* L)
{
    if (field->is_map())
        return decode_table(message, field, L);
    else if (field->is_required())
        return decode_required(message, field, L);
    else if (field->is_optional())
        return decode_optional(message, field, L);
    else if (field->is_repeated())
        return decode_repeated(message, field, L);
    else
        return false;
}

bool decode_required(const Message& message, const FieldDescriptor* field, lua_State* L)
{
    const Reflection* reflection = message.GetReflection();
    if (!reflection->HasField(message, field)) {
        proto_warn("decode_required field notFound, field=%s", field->full_name().c_str());
    }

    return decode_single(message, field, L);
}

bool decode_optional(const Message& message, const FieldDescriptor* field, lua_State* L)
{
    const Reflection* reflection = message.GetReflection();
    if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE && !reflection->HasField(message, field)) {
        lua_pushnil(L);
        return true;
    }

    return decode_single(message, field, L);
}

bool decode_repeated(const Message& message, const FieldDescriptor* field, lua_State* L)
{
    const Reflection* reflection = message.GetReflection();
    int field_size = reflection->FieldSize(message, field);
    lua_createtable(L, field_size, 0);
    for (int index = 0; index < field_size; index++)
    {
        PROTO_DO(decode_multiple(message, field, L, index));
        lua_seti(L, -2, index + 1);
    }
    return true;
}

bool decode_table(const Message& message, const FieldDescriptor* field, lua_State* L)
{
    const Reflection* reflection = message.GetReflection();
    int field_size = reflection->FieldSize(message, field);

    const Descriptor* descriptor = field->message_type();
    PROTO_ASSERT(descriptor->field_count() == 2);
    const FieldDescriptor* key = descriptor->field(0);
    const FieldDescriptor* value = descriptor->field(1);

    lua_createtable(L, 0, field_size);
    for (int index = 0; index < field_size; index++)
    {
        const Message& submessage = reflection->GetRepeatedMessage(message, field, index);
        PROTO_DO(decode_field(submessage, key, L));
        PROTO_DO(decode_field(submessage, value, L));
        lua_settable(L, -3);
    }
    return true;
}

bool decode_single(const Message& message, const FieldDescriptor* field, lua_State* L)
{
    const Reflection* reflection = message.GetReflection();
    switch (field->cpp_type())
    {
    case FieldDescriptor::CPPTYPE_DOUBLE:
        lua_pushnumber(L, reflection->GetDouble(message, field));
        break;
    case FieldDescriptor::CPPTYPE_FLOAT:
        lua_pushnumber(L, reflection->GetFloat(message, field));
        break;
    case FieldDescriptor::CPPTYPE_INT32:
        lua_pushinteger(L, reflection->GetInt32(message, field));
        break;
    case FieldDescriptor::CPPTYPE_UINT32:
        lua_pushinteger(L, reflection->GetUInt32(message, field));
        break;
    case FieldDescriptor::CPPTYPE_INT64:
        lua_pushint64(L, reflection->GetInt64(message, field));
        break;
    case FieldDescriptor::CPPTYPE_UINT64:
        lua_pushint64(L, reflection->GetUInt64(message, field));
        break;
    case FieldDescriptor::CPPTYPE_ENUM:
        lua_pushinteger(L, reflection->GetEnumValue(message, field));
        break;
    case FieldDescriptor::CPPTYPE_BOOL:
        lua_pushboolean(L, reflection->GetBool(message, field));
        break;
    case FieldDescriptor::CPPTYPE_STRING:
        {
            string value = reflection->GetString(message, field);
            lua_pushlstring(L, value.c_str(), value.size());
        }
        break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
        {
            const Message& submessage = reflection->GetMessage(message, field);
            PROTO_DO(decode_message(submessage, field->message_type(), L));
        }
        break;
    default:
        proto_error("decode_single field unknow type, field=%s", field->full_name().c_str());
        return false;
    }
    return true;
}

bool decode_multiple(const Message& message, const FieldDescriptor* field, lua_State* L, int index)
{
    const Reflection* reflection = message.GetReflection();
    switch (field->cpp_type())
    {
    case FieldDescriptor::CPPTYPE_DOUBLE:
        lua_pushnumber(L, reflection->GetRepeatedDouble(message, field, index));
        break;
    case FieldDescriptor::CPPTYPE_FLOAT:
        lua_pushnumber(L, reflection->GetRepeatedFloat(message, field, index));
        break;
    case FieldDescriptor::CPPTYPE_INT32:
        lua_pushinteger(L, reflection->GetRepeatedInt32(message, field, index));
        break;
    case FieldDescriptor::CPPTYPE_UINT32:
        lua_pushinteger(L, reflection->GetRepeatedUInt32(message, field, index));
        break;
    case FieldDescriptor::CPPTYPE_INT64:
        lua_pushint64(L, reflection->GetRepeatedInt64(message, field, index));
        break;
    case FieldDescriptor::CPPTYPE_UINT64:
        lua_pushint64(L, reflection->GetRepeatedUInt64(message, field, index));
        break;
    case FieldDescriptor::CPPTYPE_ENUM:
        lua_pushinteger(L, reflection->GetRepeatedEnumValue(message, field, index));
        break;
    case FieldDescriptor::CPPTYPE_BOOL:
        lua_pushboolean(L, reflection->GetRepeatedBool(message, field, index));
        break;
    case FieldDescriptor::CPPTYPE_STRING:
        {
            string value = reflection->GetRepeatedString(message, field, index);
            lua_pushlstring(L, value.c_str(), value.size());
        }
        break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
        {
            const Message& submessage = reflection->GetRepeatedMessage(message, field, index);
            PROTO_DO(decode_message(submessage, field->message_type(), L));
        }
        break;
    default:
        proto_error("decode_multiple field unknow type, field=%s", field->full_name().c_str());
        return false;
    }
    return true;
}

bool decode_message(const Message& message, const Descriptor* descriptor, lua_State* L)
{
    int field_count = descriptor->field_count();
    lua_createtable(L, 0, field_count);
    for (int i = 0; i < field_count; i++)
    {
        const FieldDescriptor* field = descriptor->field(i);
        PROTO_DO(decode_field(message, field, L));
        lua_setfield(L, -2, field->name().c_str());
    }
    return true;
}

bool proto_create(const char* proto, lua_State* L)
{
    const Descriptor* descriptor = g_importer->pool()->FindMessageTypeByName(proto);
    PROTO_ASSERT(descriptor);

    const Message* prototype = g_factory->GetPrototype(descriptor);
    PROTO_ASSERT(prototype);

    std::unique_ptr<Message> message(prototype->New());
    int field_count = descriptor->field_count();
    lua_createtable(L, 0, field_count);
    for (int i = 0; i < field_count; i++)
    {
        const FieldDescriptor* field = descriptor->field(i);
        if (field->is_map() || field->is_repeated())
            lua_newtable(L);
        else if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE)
            PROTO_DO(proto_create(field->message_type()->full_name().c_str(), L))
        else
            PROTO_DO(decode_single(*message.get(), field, L))
        lua_setfield(L, -2, field->name().c_str());
    }
    return true;
}

bool proto_decode(const char* proto, lua_State* L, const char* input, size_t size)
{
    const Descriptor* descriptor = g_importer->pool()->FindMessageTypeByName(proto);
    PROTO_ASSERT(descriptor);

    const Message* prototype = g_factory->GetPrototype(descriptor);
    PROTO_ASSERT(prototype);

    std::unique_ptr<Message> message(prototype->New());
    PROTO_DO(message->ParseFromArray(input, size));
    return decode_message(*message.get(), descriptor, L);
}

std::vector<const FieldDescriptor*> SortFieldsByNumber(const Descriptor* descriptor);
bool proto_unpack(const char* proto, lua_State* L, const char* input, size_t size)
{
    const Descriptor* descriptor = g_importer->pool()->FindMessageTypeByName(proto);
    PROTO_ASSERT(descriptor);

    const Message* prototype = g_factory->GetPrototype(descriptor);
    PROTO_ASSERT(prototype);

    std::unique_ptr<Message> message(prototype->New());
    PROTO_DO(message->ParseFromArray(input, size));

    std::vector<const FieldDescriptor*> fields = SortFieldsByNumber(descriptor);
    for (int i = 0; i < (int)fields.size(); i++)
    {
        const FieldDescriptor* field = fields[i];
        PROTO_DO(decode_field(*message.get(), field, L));
    }
    return true;
}