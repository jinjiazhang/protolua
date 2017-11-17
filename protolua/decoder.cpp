#include "proto.h"

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
        ProtoError("DecodeRequired field nil, field=%s\n", field->full_name().c_str());
    }

    return DecodeSingle(message, field, L);
}

bool DecodeOptional(const Message& message, const FieldDescriptor* field, lua_State* L)
{
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
    switch (field->type())
    {
    case FieldDescriptor::TYPE_DOUBLE         :  // double, exactly eight bytes on the wire.
        lua_pushnumber(L, reflection->GetDouble(message, field));
        break;
    case FieldDescriptor::TYPE_FLOAT          :  // float, exactly four bytes on the wire.
        lua_pushnumber(L, reflection->GetFloat(message, field));
        break;
    case FieldDescriptor::TYPE_INT32          :  // int32, varint on the wire.  Negative numbers
    case FieldDescriptor::TYPE_SINT32         :  // int32, ZigZag-encoded varint on the wire
    case FieldDescriptor::TYPE_SFIXED32       :  // int32, exactly four bytes on the wire
        lua_pushinteger(L, reflection->GetInt32(message, field));
        break;
    case FieldDescriptor::TYPE_UINT32         :  // uint32, varint on the wire
    case FieldDescriptor::TYPE_FIXED32        :  // uint32, exactly four bytes on the wire.
        lua_pushinteger(L, reflection->GetUInt32(message, field));
        break;
    case FieldDescriptor::TYPE_INT64          :  // int64, varint on the wire.  Negative numbers
    case FieldDescriptor::TYPE_SINT64         :  // int64, ZigZag-encoded varint on the wire
    case FieldDescriptor::TYPE_SFIXED64       :  // int64, exactly eight bytes on the wire
        lua_pushinteger(L, reflection->GetInt64(message, field));
        break;
    case FieldDescriptor::TYPE_UINT64         :  // uint64, varint on the wire.
    case FieldDescriptor::TYPE_FIXED64        :  // uint64, exactly eight bytes on the wire.
        lua_pushinteger(L, reflection->GetUInt64(message, field));
        break;
    case FieldDescriptor::TYPE_ENUM           :  // Enum, varint on the wire
        lua_pushinteger(L, reflection->GetEnumValue(message, field));
        break;
    case FieldDescriptor::TYPE_BOOL           :  // bool, varint on the wire.
        lua_pushboolean(L, reflection->GetBool(message, field));
        break;
    case FieldDescriptor::TYPE_BYTES          :  // Arbitrary byte array.
    case FieldDescriptor::TYPE_STRING         :  // UTF-8 text.
        {
            string& value = reflection->GetString(message, field);
            lua_pushlstring(L, value.c_str(), value.size());
        }
        break;
    case FieldDescriptor::TYPE_MESSAGE        :  // Length-delimited message.
        {
            const Message& submessage = reflection->GetMessage(message, field);
            PROTO_DO(DecodeMessage(submessage, field->message_type(), L));
        }
        break;
    case FieldDescriptor::TYPE_GROUP          :  // Tag-delimited message.  Deprecated.
    default:
        return false;
    }
    return true;
}

bool DecodeMultiple(const Message& message, const FieldDescriptor* field, lua_State* L, int index)
{
    const Reflection* reflection = message.GetReflection();
    switch (field->type())
    {
    case FieldDescriptor::TYPE_DOUBLE         :  // double, exactly eight bytes on the wire.
        lua_pushnumber(L, reflection->GetRepeatedDouble(message, field, index));
        break;
    case FieldDescriptor::TYPE_FLOAT          :  // float, exactly four bytes on the wire.
        lua_pushnumber(L, reflection->GetRepeatedFloat(message, field, index));
        break;
    case FieldDescriptor::TYPE_INT32          :  // int32, varint on the wire.  Negative numbers
    case FieldDescriptor::TYPE_SINT32         :  // int32, ZigZag-encoded varint on the wire
    case FieldDescriptor::TYPE_SFIXED32       :  // int32, exactly four bytes on the wire
        lua_pushinteger(L, reflection->GetRepeatedInt32(message, field, index));
        break;
    case FieldDescriptor::TYPE_UINT32         :  // uint32, varint on the wire
    case FieldDescriptor::TYPE_FIXED32        :  // uint32, exactly four bytes on the wire.
        lua_pushinteger(L, reflection->GetRepeatedUInt32(message, field, index));
        break;
    case FieldDescriptor::TYPE_INT64          :  // int64, varint on the wire.  Negative numbers
    case FieldDescriptor::TYPE_SINT64         :  // int64, ZigZag-encoded varint on the wire
    case FieldDescriptor::TYPE_SFIXED64       :  // int64, exactly eight bytes on the wire
        lua_pushinteger(L, reflection->GetRepeatedInt64(message, field, index));
        break;
    case FieldDescriptor::TYPE_UINT64         :  // uint64, varint on the wire.
    case FieldDescriptor::TYPE_FIXED64        :  // uint64, exactly eight bytes on the wire.
        lua_pushinteger(L, reflection->GetRepeatedUInt64(message, field, index));
        break;
    case FieldDescriptor::TYPE_ENUM           :  // Enum, varint on the wire
        lua_pushinteger(L, reflection->GetRepeatedEnumValue(message, field, index));
        break;
    case FieldDescriptor::TYPE_BOOL           :  // bool, varint on the wire.
        lua_pushboolean(L, reflection->GetRepeatedBool(message, field, index));
        break;
    case FieldDescriptor::TYPE_BYTES          :  // Arbitrary byte array.
    case FieldDescriptor::TYPE_STRING         :  // UTF-8 text.
        {
            string& value = reflection->GetRepeatedString(message, field, index);
            lua_pushlstring(L, value.c_str(), value.size());
        }
        break;
    case FieldDescriptor::TYPE_MESSAGE        :  // Length-delimited message.
        {
            const Message& submessage = reflection->GetRepeatedMessage(message, field, index);
            PROTO_DO(DecodeMessage(submessage, field->message_type(), L));
        }
        break;
    case FieldDescriptor::TYPE_GROUP          :  // Tag-delimited message.  Deprecated.
    default:
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
        lua_pushstring(L, field->name().c_str());
        PROTO_DO(DecodeField(message, field, L));
        lua_settable(L, -3);
    }
    return true;
}

bool ProtoDecode(const char* proto, lua_State* L, const char* input, size_t size)
{
    const Descriptor* descriptor = g_descriptor_pool->FindMessageTypeByName(proto);
    PROTO_ASSERT(descriptor);

    const Message* prototype = g_message_factory->GetPrototype(descriptor);
    PROTO_ASSERT(prototype);

    Message* message = prototype->New();
    PROTO_ASSERT(message);

    message->ParseFromArray(input, size);
    return DecodeMessage(*message, descriptor, L);
}

bool ProtoUnpack(const char* proto, lua_State* L, const char* input, size_t size)
{
    const Descriptor* descriptor = g_descriptor_pool->FindMessageTypeByName(proto);
    PROTO_ASSERT(descriptor);

    const Message* prototype = g_message_factory->GetPrototype(descriptor);
    PROTO_ASSERT(prototype);

    Message* message = prototype->New();
    PROTO_ASSERT(message);

    message->ParseFromArray(input, size);

    std::vector<const FieldDescriptor*> fields = SortFieldsByNumber(descriptor);
    for (int i = 0; i < (int)fields.size(); i++)
    {
        const FieldDescriptor* field = fields[i];
        PROTO_DO(DecodeField(*message, field, L));
    }
    return true;
}