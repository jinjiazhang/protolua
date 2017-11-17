#include "proto.h"

bool DecodeField(CodedInputStream* input, const FieldDescriptor* field, lua_State* L, uint32 tag);
bool DecodeSingle(CodedInputStream* input, const FieldDescriptor* field, lua_State* L, uint32 tag);
bool DecodeRepeated(CodedInputStream* input, const FieldDescriptor* field, lua_State* L, uint32 tag);
bool DecodeNumber(CodedInputStream* input, const FieldDescriptor* field, lua_State* L, uint32 tag);
bool DecodeInteger(CodedInputStream* input, const FieldDescriptor* field, lua_State* L, uint32 tag);
bool DecodeBoolean(CodedInputStream* input, const FieldDescriptor* field, lua_State* L, uint32 tag);
bool DecodeString(CodedInputStream* input, const FieldDescriptor* field, lua_State* L, uint32 tag);
bool DecodeTable(CodedInputStream* input, const FieldDescriptor* field, lua_State* L, uint32 tag);
bool DecodeMessage(CodedInputStream* input, const FieldDescriptor* field, lua_State* L, uint32 tag);

bool DefaultField(CodedInputStream* input, const FieldDescriptor* field, lua_State* L);
bool DefaultMessage(CodedInputStream* input, const FieldDescriptor* field, lua_State* L);
bool DecodeMessage(const Descriptor* message, CodedInputStream* input, lua_State* L);
bool UnpackMessage(const Descriptor* message, CodedInputStream* input, lua_State* L);


bool DefaultField(CodedInputStream* input, const FieldDescriptor* field, lua_State* L)
{
    if (field->is_map() || field->is_repeated())
    {
        lua_newtable(L);
        return true;
    }

    switch (field->type())
    {
    case FieldDescriptor::TYPE_DOUBLE         :  // double, exactly eight bytes on the wire.
        lua_pushnumber(L, field->default_value_double());
        break;
    case FieldDescriptor::TYPE_FLOAT          :  // float, exactly four bytes on the wire.
        lua_pushnumber(L, field->default_value_float());
        break;
    case FieldDescriptor::TYPE_INT64          :  // int64, varint on the wire.  Negative numbers
    case FieldDescriptor::TYPE_SFIXED64       :  // int64, exactly eight bytes on the wire
    case FieldDescriptor::TYPE_SINT64         :  // int64, ZigZag-encoded varint on the wire
        lua_pushinteger(L, field->default_value_int64());
        break;
    case FieldDescriptor::TYPE_INT32          :  // int32, varint on the wire.  Negative numbers
    case FieldDescriptor::TYPE_SFIXED32       :  // int32, exactly four bytes on the wire
    case FieldDescriptor::TYPE_SINT32         :  // int32, ZigZag-encoded varint on the wire
        lua_pushinteger(L, field->default_value_int32());
        break;
    case FieldDescriptor::TYPE_UINT64         :  // uint64, varint on the wire.
    case FieldDescriptor::TYPE_FIXED64        :  // uint64, exactly eight bytes on the wire.
        lua_pushinteger(L, field->default_value_uint64());
        break;
    case FieldDescriptor::TYPE_UINT32         :  // uint32, varint on the wire
    case FieldDescriptor::TYPE_FIXED32        :  // uint32, exactly four bytes on the wire.
        lua_pushinteger(L, field->default_value_uint32());
        break;
    case FieldDescriptor::TYPE_ENUM           :  // Enum, varint on the wire
        lua_pushinteger(L, field->default_value_enum()->number());
        break;
    case FieldDescriptor::TYPE_BOOL           :  // bool, varint on the wire.
        lua_pushboolean(L, field->default_value_bool());
        break;
    case FieldDescriptor::TYPE_BYTES          :  // Arbitrary byte array.
    case FieldDescriptor::TYPE_STRING         :  // UTF-8 text.
        lua_pushlstring(L, field->default_value_string().c_str(), field->default_value_string().size());
        break;
    case FieldDescriptor::TYPE_MESSAGE        :  // Length-delimited message.
        PROTO_DO(DefaultMessage(input, field, L));
        break;
    case FieldDescriptor::TYPE_GROUP          :  // Tag-delimited message.  Deprecated.
    default:
        return false;
    }
    return true;
}

bool DefaultMessage(CodedInputStream* input, const FieldDescriptor* field, lua_State* L)
{
    const Descriptor* descriptor = field->message_type();
    PROTO_ASSERT(descriptor);

    lua_newtable(L);
    for (int i = 0; i < descriptor->field_count(); i++)
    {
        const FieldDescriptor* field = descriptor->field(i);
        lua_pushstring(L, field->name().c_str());
        PROTO_DO(DefaultField(input, field, L));
        lua_settable(L, -3);
    }
    return true;
}

bool DecodeField(CodedInputStream* input, const FieldDescriptor* field, lua_State* L, uint32 tag)
{
    if (field->is_map())
        return DecodeTable(input, field, L, tag);
    else if (field->is_required())
        return DecodeSingle(input, field, L, tag);
    else if (field->is_optional())
        return DecodeSingle(input, field, L, tag);
    else if (field->is_repeated())
        return DecodeRepeated(input, field, L, tag);
    else
        return false;
}

bool DecodeSingle(CodedInputStream* input, const FieldDescriptor* field, lua_State* L, uint32 tag)
{
    switch (field->type())
    {
    case FieldDescriptor::TYPE_DOUBLE         :  // double, exactly eight bytes on the wire.
    case FieldDescriptor::TYPE_FLOAT          :  // float, exactly four bytes on the wire.
        return DecodeNumber(input, field, L, tag);
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
        return DecodeInteger(input, field, L, tag);
    case FieldDescriptor::TYPE_BOOL           :  // bool, varint on the wire.
        return DecodeBoolean(input, field, L, tag);
    case FieldDescriptor::TYPE_BYTES          :  // Arbitrary byte array.
    case FieldDescriptor::TYPE_STRING         :  // UTF-8 text.
        return DecodeString(input, field, L, tag);
    case FieldDescriptor::TYPE_MESSAGE        :  // Length-delimited message.
        return DecodeMessage(input, field, L, tag);
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
    
bool DecodeNumber(CodedInputStream* input, const FieldDescriptor* field, lua_State* L, uint32 tag)
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

bool DecodeInteger(CodedInputStream* input, const FieldDescriptor* field, lua_State* L, uint32 tag)
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

bool DecodeBoolean(CodedInputStream* input, const FieldDescriptor* field, lua_State* L, uint32 tag)
{
    bool value;
    PROTO_DO((WireFormatLite::ReadPrimitive<bool, WireFormatLite::TYPE_BOOL>(input, &value)));
    lua_pushboolean(L, value);
    return true;
}

bool DecodeString(CodedInputStream* input, const FieldDescriptor* field, lua_State* L, uint32 tag)
{
    string value;
    PROTO_DO(WireFormatLite::ReadString(input, &value));
    lua_pushlstring(L, value.c_str(), value.size());
    return true;
}

bool DecodeRepeated(CodedInputStream* input, const FieldDescriptor* field, lua_State* L, uint32 tag)
{
    lua_Integer index = 1;
    lua_newtable(L);

    WireFormatLite::WireType type = WireFormatLite::GetTagWireType(tag);
    if (field->is_packable() && type == WireFormatLite::WIRETYPE_LENGTH_DELIMITED) // [packed=true]
    {
        int length;
        PROTO_DO(input->ReadVarintSizeAsInt(&length));
        CodedInputStream::Limit limit = input->PushLimit(length);
        while (input->BytesUntilLimit() > 0) {
            PROTO_DO(DecodeSingle(input, field, L, tag));
            lua_seti(L, -2, index);
            ++index;
        }
        input->PopLimit(limit);
    }
    else // [packed=false]
    {
        do 
        {
            PROTO_DO(DecodeSingle(input, field, L, tag));
            lua_seti(L, -2, index);
            ++index;
        } while (input->ExpectTag(tag));
    }
    return true;
}

bool DecodeTable(CodedInputStream* input, const FieldDescriptor* field, lua_State* L, uint32 tag)
{
    const Descriptor* descriptor = field->message_type();
    PROTO_ASSERT(descriptor);
    
    const FieldDescriptor* key = descriptor->field(0);
    const FieldDescriptor* value = descriptor->field(1);

    int length;
    uint32 key_tag;
    uint32 value_tag;
    lua_newtable(L);
    do 
    {
        PROTO_DO(input->ReadVarintSizeAsInt(&length));
        CodedInputStream::Limit limit = input->PushLimit(length);
        key_tag = input->ReadTagNoLastTag();
        PROTO_ASSERT(key_tag==WireFormat::MakeTag(key));
        PROTO_DO(DecodeSingle(input, key, L, key_tag));
        value_tag = input->ReadTagNoLastTag();
        PROTO_ASSERT(value_tag==WireFormat::MakeTag(value));
        PROTO_DO(DecodeSingle(input, value, L, value_tag));
        lua_settable(L, -3);
        input->PopLimit(limit);
    } while (input->ExpectTag(tag));
    return true;
}

bool DecodeMessage(CodedInputStream* input, const FieldDescriptor* field, lua_State* L, uint32 tag)
{
    const Descriptor* descriptor = field->message_type();
    PROTO_ASSERT(descriptor);

    int length;
    PROTO_DO(input->ReadVarintSizeAsInt(&length));
    CodedInputStream::Limit limit = input->PushLimit(length);
    PROTO_DO(DecodeMessage(descriptor, input, L));
    input->PopLimit(limit);
    return true;
}

bool DecodeMessage(const Descriptor* message, CodedInputStream* input, lua_State* L)
{
    uint32 tag;
    std::set<int> numbers;
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

        // PROTO_ASSERT(tag==WireFormat::MakeTag(field));
        lua_pushstring(L, field->name().c_str());
        PROTO_DO(DecodeField(input, field, L, tag));
        lua_settable(L, -3);
        numbers.insert(field->number());
    }

    for (int i = 0; i < message->field_count(); i++)
    {
        const FieldDescriptor* field = message->field(i);
        if (numbers.find(field->number()) == numbers.end())
        {
            lua_pushstring(L, field->name().c_str());
            PROTO_DO(DefaultField(input, field, L));
            lua_settable(L, -3);
        }
    }
    return true;
}

bool AdjustStack(std::list<int>& sequence, lua_State* L, int number)
{
    int index = -1;
    std::list<int>::reverse_iterator it = sequence.rbegin();
    for (; it != sequence.rend(); ++it)
    {
        if (*it < number)
            break;
        --index;
    }

    sequence.insert(it.base(), number);
    lua_insert(L, index);
    return true;
}

bool UnpackMessage(const Descriptor* message, CodedInputStream* input, lua_State* L)
{
    uint32 tag;
    std::set<int> numbers;
    std::list<int> sequence;
    while (tag = input->ReadTagNoLastTag())
    {
        int number = WireFormatLite::GetTagFieldNumber(tag);
        const FieldDescriptor* field = message->FindFieldByNumber(number);
        if (field == NULL) // skip unknow field
        {
            WireFormat::SkipField(input, tag, NULL);
            continue;
        }

        // PROTO_ASSERT(tag==WireFormat::MakeTag(field));
        PROTO_DO(DecodeField(input, field, L, tag));
        PROTO_DO(AdjustStack(sequence, L, number));
        numbers.insert(field->number());
    }

    for (int i = 0; i < message->field_count(); i++)
    {
        const FieldDescriptor* field = message->field(i);
        if (numbers.find(field->number()) == numbers.end())
        {
            PROTO_DO(DefaultField(input, field, L));
            PROTO_DO(AdjustStack(sequence, L, field->number()));
        }
    }
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
    const Descriptor* message = g_descriptor_pool->FindMessageTypeByName(proto);
    PROTO_ASSERT(message);

    ArrayInputStream buffer((void*)input, (int)size);
    CodedInputStream stream(&buffer);
    return UnpackMessage(message, &stream, L);
}