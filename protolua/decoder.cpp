#include "proto.h"

bool DecodeField(const FieldDescriptor* field, CodedInputStream* input, lua_State* L, uint32 tag);
bool DecodeSingle(const FieldDescriptor* field, CodedInputStream* input, lua_State* L, uint32 tag);
bool DecodeRepeated(const FieldDescriptor* field, CodedInputStream* input, lua_State* L, uint32 tag);
bool DecodeNumber(const FieldDescriptor* field, CodedInputStream* input, lua_State* L, uint32 tag);
bool DecodeInteger(const FieldDescriptor* field, CodedInputStream* input, lua_State* L, uint32 tag);
bool DecodeBoolean(const FieldDescriptor* field, CodedInputStream* input, lua_State* L, uint32 tag);
bool DecodeString(const FieldDescriptor* field, CodedInputStream* input, lua_State* L, uint32 tag);
bool DecodeTable(const FieldDescriptor* field, CodedInputStream* input, lua_State* L, uint32 tag);
bool DecodeMessage(const FieldDescriptor* field, CodedInputStream* input, lua_State* L, uint32 tag);

bool DecodeField(const FieldDescriptor* field, CodedInputStream* input, lua_State* L, uint32 tag)
{
    if (field->is_map())
        return DecodeTable(field, input, L, tag);
    else if (field->is_required())
        return DecodeSingle(field, input, L, tag);
    else if (field->is_optional())
        return DecodeSingle(field, input, L, tag);
    else if (field->is_repeated())
        return DecodeRepeated(field, input, L, tag);
    else
        return false;
}

bool DecodeSingle(const FieldDescriptor* field, CodedInputStream* input, lua_State* L, uint32 tag)
{
    switch (field->type())
    {
    case FieldDescriptor::TYPE_DOUBLE         :  // double, exactly eight bytes on the wire.
    case FieldDescriptor::TYPE_FLOAT          :  // float, exactly four bytes on the wire.
        return DecodeNumber(field, input, L, tag);
    case FieldDescriptor::TYPE_INT64          :  // int64, varint on the wire.  Negative numbers
    case FieldDescriptor::TYPE_UINT64         :  // uint64, varint on the wire.
    case FieldDescriptor::TYPE_INT32          :  // int32, varint on the wire.  Negative numbers
    case FieldDescriptor::TYPE_UINT32         :  // uint32, varint on the wire
    case FieldDescriptor::TYPE_FIXED64        :  // uint64, exactly eight bytes on the wire.
    case FieldDescriptor::TYPE_FIXED32        :  // uint32, exactly four bytes on the wire.
    case FieldDescriptor::TYPE_SFIXED32       :  // int32, exactly four bytes on the wire
    case FieldDescriptor::TYPE_SFIXED64       :  // int64, exactly eight bytes on the wire
    case FieldDescriptor::TYPE_SINT32         :  // int32, ZigZag-encoded varint on the wire
    case FieldDescriptor::TYPE_SINT64         :  // int64, ZigZag-encoded varint on the wire
    case FieldDescriptor::TYPE_ENUM           :  // Enum, varint on the wire
        return DecodeInteger(field, input, L, tag);
    case FieldDescriptor::TYPE_BOOL           :  // bool, varint on the wire.
        return DecodeBoolean(field, input, L, tag);
    case FieldDescriptor::TYPE_BYTES          :  // Arbitrary byte array.
    case FieldDescriptor::TYPE_STRING         :  // UTF-8 text.
        return DecodeString(field, input, L, tag);
    case FieldDescriptor::TYPE_MESSAGE        :  // Length-delimited message.
        return DecodeMessage(field, input, L, tag);
    case FieldDescriptor::TYPE_GROUP          :  // Tag-delimited message.  Deprecated.
    default:
        return false;
    }
    return true;
}

#define PROTO_READ_PRIMITIVE(CType, WType, input, value) \
    do { \
        CType temp; \
        PROTO_DO((WireFormatLite::ReadPrimitive<CType, WType>((input), &temp))); \
        *(value) = temp; \
    } while (0);
    
bool DecodeNumber(const FieldDescriptor* field, CodedInputStream* input, lua_State* L, uint32 tag)
{
    lua_Number value;
    switch (field->type())
    {
    case FieldDescriptor::TYPE_DOUBLE         :  // double, exactly eight bytes on the wire.
        PROTO_READ_PRIMITIVE(double, WireFormatLite::TYPE_DOUBLE, input, &value);
        break;
    case FieldDescriptor::TYPE_FLOAT          :  // float, exactly four bytes on the wire.
        PROTO_READ_PRIMITIVE(float, WireFormatLite::TYPE_FLOAT, input, &value);
        break;
    default:
        return false;
    }

    lua_pushnumber(L, value);
    return true;
}

bool DecodeInteger(const FieldDescriptor* field, CodedInputStream* input, lua_State* L, uint32 tag)
{
    lua_Integer value;
    switch (field->type())
    {
    case FieldDescriptor::TYPE_INT64          :  // int64, varint on the wire.  Negative numbers
        PROTO_READ_PRIMITIVE(int64, WireFormatLite::TYPE_INT64, input, &value);
        break;
    case FieldDescriptor::TYPE_UINT64         :  // uint64, varint on the wire.
        PROTO_READ_PRIMITIVE(uint64, WireFormatLite::TYPE_UINT64, input, &value);
        break;
    case FieldDescriptor::TYPE_INT32          :  // int32, varint on the wire.  Negative numbers
        PROTO_READ_PRIMITIVE(int32, WireFormatLite::TYPE_INT32, input, &value);
        break;
    case FieldDescriptor::TYPE_UINT32         :  // uint32, varint on the wire
        PROTO_READ_PRIMITIVE(uint32, WireFormatLite::TYPE_UINT32, input, &value);
        break;
    case FieldDescriptor::TYPE_FIXED64        :  // uint64, exactly eight bytes on the wire.
        PROTO_READ_PRIMITIVE(uint64, WireFormatLite::TYPE_FIXED64, input, &value);
        break;
    case FieldDescriptor::TYPE_FIXED32        :  // uint32, exactly four bytes on the wire.
        PROTO_READ_PRIMITIVE(uint32, WireFormatLite::TYPE_FIXED32, input, &value);
        break;
    case FieldDescriptor::TYPE_SFIXED32       :  // int32, exactly four bytes on the wire
        PROTO_READ_PRIMITIVE(int32, WireFormatLite::TYPE_SFIXED32, input, &value);
        break;
    case FieldDescriptor::TYPE_SFIXED64       :  // int64, exactly eight bytes on the wire
        PROTO_READ_PRIMITIVE(int64, WireFormatLite::TYPE_SFIXED64, input, &value);
        break;
    case FieldDescriptor::TYPE_SINT32         :  // int32, ZigZag-encoded varint on the wire
        PROTO_READ_PRIMITIVE(int32, WireFormatLite::TYPE_SINT32, input, &value);
        break;
    case FieldDescriptor::TYPE_SINT64         :  // int64, ZigZag-encoded varint on the wire
        PROTO_READ_PRIMITIVE(int64, WireFormatLite::TYPE_SINT64, input, &value);
        break;
    case FieldDescriptor::TYPE_ENUM           :  // Enum, varint on the wire
        PROTO_READ_PRIMITIVE(int, WireFormatLite::TYPE_ENUM, input, &value);
        break;
    default:
        return false;
    }

    lua_pushinteger(L, value);
    return true;
}

bool DecodeBoolean(const FieldDescriptor* field, CodedInputStream* input, lua_State* L, uint32 tag)
{
    bool value;
    PROTO_DO((WireFormatLite::ReadPrimitive<bool, WireFormatLite::TYPE_BOOL>(input, &value)));
    lua_pushboolean(L, value);
    return true;
}

bool DecodeString(const FieldDescriptor* field, CodedInputStream* input, lua_State* L, uint32 tag)
{
    string value;
    PROTO_DO(WireFormatLite::ReadString(input, &value));
    lua_pushlstring(L, value.c_str(), value.size());
    return true;
}

bool DecodeRepeated(const FieldDescriptor* field, CodedInputStream* input, lua_State* L, uint32 tag)
{
    lua_Integer index = 1;
    lua_newtable(L);
    if (field->is_packed()) // tag, length, value1, value2, value3, ...
    {
        int length;
        PROTO_DO(input->ReadVarintSizeAsInt(&length));
        CodedInputStream::Limit limit = input->PushLimit(length);
        while (input->BytesUntilLimit() > 0) {
            PROTO_DO(DecodeSingle(field, input, L, tag));
            lua_seti(L, -2, index);
            index++;
        }
        input->PopLimit(limit);
    }
    else // tag, value1, tag, value2, tag, value3, ... ...
    {
        do 
        {
            PROTO_DO(DecodeSingle(field, input, L, tag));
            lua_seti(L, -2, index);
            index++;
        } while (input->ExpectTag(tag));
    }
    return true;
}

bool DecodeTable(const FieldDescriptor* field, CodedInputStream* input, lua_State* L, uint32 tag)
{
    return true;
}

bool DecodeMessage(const Descriptor* message, CodedInputStream* input, lua_State* L)
{
    uint32 tag;
    lua_newtable(L);
    while (tag = input->ReadTagNoLastTag())
    {
        int number = WireFormatLite::GetTagFieldNumber(tag);
        const FieldDescriptor* field = message->FindFieldByNumber(number);
        if (field == NULL) // skip unknow field
        {
            WireFormat::SkipField(input, tag, NULL);
            continue;
        }

        PROTO_ASSERT(tag==WireFormat::MakeTag(field));
        lua_pushstring(L, field->name().c_str());
        PROTO_DO(DecodeField(field, input, L, tag));
        lua_settable(L, -3);
    }
    return true;
}

bool DecodeMessage(const FieldDescriptor* field, CodedInputStream* input, lua_State* L, uint32 tag)
{
    string type_name = field->message_type()->full_name();
    const Descriptor* message = g_descriptor_pool->FindMessageTypeByName(type_name);
    PROTO_ASSERT(message);

    int length;
    PROTO_DO(input->ReadVarintSizeAsInt(&length));
    CodedInputStream::Limit limit = input->PushLimit(length);
    PROTO_DO(DecodeMessage(message, input, L));
    input->PopLimit(limit);
    return true;
}

bool ProtoDecode(const char* proto, lua_State* L, const char* input, size_t size)
{
    const Descriptor* message = g_descriptor_pool->FindMessageTypeByName(proto);
    PROTO_ASSERT(message);

    ArrayInputStream buffer((void*)input, (int)size);
    CodedInputStream stream(&buffer);
    return DecodeMessage(message, &stream, L);
}

bool ProtoUnpack(const char* proto, lua_State* L, const char* input, size_t size)
{
    return 0;
}