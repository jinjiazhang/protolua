#include "protolua.h"

bool DecodeField(const Message& message, const FieldDescriptor* field, lua_State* L);
bool DecodeRequired(const Message& message, const FieldDescriptor* field, lua_State* L);
bool DecodeOptional(const Message& message, const FieldDescriptor* field, lua_State* L);
bool DecodeRepeated(const Message& message, const FieldDescriptor* field, lua_State* L);
bool DecodeTable(const Message& message, const FieldDescriptor* field, lua_State* L);
bool DecodeSingle(const Message& message, const FieldDescriptor* field, lua_State* L);
bool DecodeMultiple(const Message& message, const FieldDescriptor* field, lua_State* L, int index);
bool DecodeMessage(const Message& message, const Descriptor* descriptor, lua_State* L);

bool DecodeField(const Message& message, const FieldDescriptor* field, lua_State* L)
{
    if (field->is_map())
        return DecodeTable(message, field, L);
    else if (field->is_required())
        return DecodeRequired(message, field, L);
    else if (field->is_optional())
        return DecodeOptional(message, field, L);
    else if (field->is_repeated())
        return DecodeRepeated(message, field, L);
    else
        return false;
}

bool DecodeRequired(const Message& message, const FieldDescriptor* field, lua_State* L)
{
    const Reflection* reflection = message.GetReflection();
    if (!reflection->HasField(message, field)) {
        proto_warn("DecodeRequired field notFound, field=%s\n", field->full_name().c_str());
    }

    return DecodeSingle(message, field, L);
}

bool DecodeOptional(const Message& message, const FieldDescriptor* field, lua_State* L)
{
    const Reflection* reflection = message.GetReflection();
    if (field->containing_oneof() && !reflection->HasField(message, field)) {
        lua_pushnil(L); // oneof field special
        return true;
    }

    return DecodeSingle(message, field, L);
}

bool DecodeRepeated(const Message& message, const FieldDescriptor* field, lua_State* L)
{
    const Reflection* reflection = message.GetReflection();
    int field_size = reflection->FieldSize(message, field);
    
    lua_newtable(L);
    for (int index = 0; index < field_size; index++)
    {
        PROTO_DO(DecodeMultiple(message, field, L, index));
        lua_seti(L, -2, index + 1);
    }
    return true;
}

bool DecodeTable(const Message& message, const FieldDescriptor* field, lua_State* L)
{
    const Reflection* reflection = message.GetReflection();
    int field_size = reflection->FieldSize(message, field);

    const Descriptor* descriptor = field->message_type();
    PROTO_ASSERT(descriptor->field_count() == 2);
    const FieldDescriptor* key = descriptor->field(0);
    const FieldDescriptor* value = descriptor->field(1);

    lua_newtable(L);
    for (int index = 0; index < field_size; index++)
    {
        const Message& submessage = reflection->GetRepeatedMessage(message, field, index);
        PROTO_DO(DecodeField(submessage, key, L));
        PROTO_DO(DecodeField(submessage, value, L));
        lua_settable(L, -3);
    }
    return true;
}

bool DecodeSingle(const Message& message, const FieldDescriptor* field, lua_State* L)
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
        lua_pushinteger(L, reflection->GetInt64(message, field));
        break;
    case FieldDescriptor::CPPTYPE_UINT64:
        lua_pushinteger(L, reflection->GetUInt64(message, field));
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
            PROTO_DO(DecodeMessage(submessage, field->message_type(), L));
        }
        break;
    default:
        proto_error("DecodeSingle field unknow type, field=%s\n", field->full_name().c_str());
        return false;
    }
    return true;
}

bool DecodeMultiple(const Message& message, const FieldDescriptor* field, lua_State* L, int index)
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
        lua_pushinteger(L, reflection->GetRepeatedInt64(message, field, index));
        break;
    case FieldDescriptor::CPPTYPE_UINT64:
        lua_pushinteger(L, reflection->GetRepeatedUInt64(message, field, index));
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
            PROTO_DO(DecodeMessage(submessage, field->message_type(), L));
        }
        break;
    default:
        proto_error("DecodeMultiple field unknow type, field=%s\n", field->full_name().c_str());
        return false;
    }
    return true;
}

bool DecodeMessage(const Message& message, const Descriptor* descriptor, lua_State* L)
{
    lua_newtable(L);
    for (int i = 0; i < descriptor->field_count(); i++)
    {
        const FieldDescriptor* field = descriptor->field(i);
        PROTO_DO(DecodeField(message, field, L));
        lua_setfield(L, -2, field->name().c_str());
    }
    return true;
}

bool ProtoDecode(const char* proto, lua_State* L, const char* input, size_t size)
{
    const Descriptor* descriptor = g_importer.pool()->FindMessageTypeByName(proto);
    PROTO_ASSERT(descriptor);

    const Message* prototype = g_factory.GetPrototype(descriptor);
    PROTO_ASSERT(prototype);

    google::protobuf::scoped_ptr<Message> message(prototype->New());
    PROTO_DO(message->ParseFromArray(input, size));
    return DecodeMessage(*message.get(), descriptor, L);
}

std::vector<const FieldDescriptor*> SortFieldsByNumber(const Descriptor* descriptor);
bool ProtoUnpack(const char* proto, lua_State* L, const char* input, size_t size)
{
    const Descriptor* descriptor = g_importer.pool()->FindMessageTypeByName(proto);
    PROTO_ASSERT(descriptor);

    const Message* prototype = g_factory.GetPrototype(descriptor);
    PROTO_ASSERT(prototype);

    google::protobuf::scoped_ptr<Message> message(prototype->New());
    PROTO_DO(message->ParseFromArray(input, size));

    std::vector<const FieldDescriptor*> fields = SortFieldsByNumber(descriptor);
    for (int i = 0; i < (int)fields.size(); i++)
    {
        const FieldDescriptor* field = fields[i];
        PROTO_DO(DecodeField(*message.get(), field, L));
    }
    return true;
}