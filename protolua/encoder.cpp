#include "proto.h"

bool EncodeField(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index);
bool EncodeRequired(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index);
bool EncodeOptional(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index);
bool EncodeRepeated(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index);
bool EncodeNumber(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index);
bool EncodeInteger(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index);
bool EncodeBoolean(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index);
bool EncodeString(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index);
bool EncodeMessage(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index);
bool EncodeTable(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index);
bool EncodeSingle(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index);

const int k_skip_count = 4;
const char* SkipLength(CodedOutputStream* stream)
{
    size_t length = 0;
    const char* position = NULL;
    stream->Skip(k_skip_count);
    stream->GetDirectBufferPointer((void**)&position, (int*)&length);
    return position;
}

bool FillLength(CodedOutputStream* output, const char* last_pos)
{
    size_t length = 0;
    const char* position = NULL;
    output->GetDirectBufferPointer((void**)&position, (int*)&length);
    int32 value = position - last_pos;
    PROTO_ASSERT(CodedOutputStream::VarintSize32(value) <= k_skip_count);
    
    // fill fiexd length Varint32
    uint8* target = (uint8*)last_pos-k_skip_count;
    for (int i = 0; i < k_skip_count; i++)
        *(target+i) = static_cast<uint8>(value >> (i*7) | 0x80);
    *(target+k_skip_count-1) &= 0x7F;
    return true;
}

bool EncodeField(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index)
{
    if (field->is_map())
        return EncodeTable(field, output, L, index);
    else if (field->is_required())
        return EncodeRequired(field, output, L, index);
    else if (field->is_optional())
        return EncodeOptional(field, output, L, index);
    else if (field->is_repeated())
        return EncodeRepeated(field, output, L, index);
    else
        return false;
}

bool EncodeRequired(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index)
{
    if (lua_isnil(L, index))
        return false;
    return EncodeSingle(field, output, L, index);
}

bool EncodeOptional(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index)
{
    if (lua_isnil(L, index))
        return true;
    return EncodeSingle(field, output, L, index);
}

bool EncodeRepeated(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index)
{
    if (lua_isnil(L, index)) return true;
    if (!lua_istable(L, index)) return false;

    int count = (int)luaL_len(L, index);
    if (count == 0) return true;
    if (field->is_packed()) // tag, length, value1, value2, value3, ...
    {
        WireFormatLite::WriteTag(field->number(), WireFormatLite::WIRETYPE_LENGTH_DELIMITED, output);
        const char* position = SkipLength(output);
        for (int i = 0; i < count; i++)
        {
            lua_geti(L, index, i + 1);
            PROTO_DO(EncodeSingle(field, output, L, index + 1)); // encode with tag
            lua_remove(L, index + 1);
        }
        PROTO_DO(FillLength(output, position));
    }
    else // tag, value1, tag, value2, tag, value3, ... ...
    {
        for (int i = 0; i < count; i++)
        {
            lua_geti(L, index, i + 1);
            PROTO_DO(EncodeSingle(field, output, L, index + 1)); // encode no tag
            lua_remove(L, index + 1);
        }
    }
    return true;
}

bool EncodeSingle(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index)
{
    switch (field->type())
    {
    case FieldDescriptor::TYPE_DOUBLE         :  // double, exactly eight bytes on the wire.
    case FieldDescriptor::TYPE_FLOAT          :  // float, exactly four bytes on the wire.
        return EncodeNumber(field, output, L, index);
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
        return EncodeInteger(field, output, L, index);
    case FieldDescriptor::TYPE_BOOL           :  // bool, varint on the wire.
        return EncodeBoolean(field, output, L, index);
    case FieldDescriptor::TYPE_BYTES          :  // Arbitrary byte array.
    case FieldDescriptor::TYPE_STRING         :  // UTF-8 text.
        return EncodeString(field, output, L, index);
    case FieldDescriptor::TYPE_MESSAGE        :  // Length-delimited message.
        return EncodeMessage(field, output, L, index);
    case FieldDescriptor::TYPE_GROUP          :  // Tag-delimited message.  Deprecated.
    default:
        return false;
    }
    return true;
}

bool EncodeNumber(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index)
{
    lua_Number value = lua_tonumber(L, index);
    switch (field->type())
    {
    case FieldDescriptor::TYPE_DOUBLE         :  // double, exactly eight bytes on the wire.
        field->is_packed() ?  WireFormatLite::WriteDoubleNoTag((double)value, output) 
            : WireFormatLite::WriteDouble(field->number(), (double)value, output);
        break;
    case FieldDescriptor::TYPE_FLOAT          :  // float, exactly four bytes on the wire.
        field->is_packed() ? WireFormatLite::WriteFloatNoTag((float)value, output) 
            : WireFormatLite::WriteFloat(field->number(), (float)value, output);
        break;
    default:
        return false;
    }
    return true;
}

bool EncodeInteger(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index)
{
    lua_Integer value = lua_tointeger(L, index);
    switch (field->type())
    {
    case FieldDescriptor::TYPE_INT64          :  // int64, varint on the wire.  Negative numbers
        field->is_packed() ? WireFormatLite::WriteInt64NoTag((int64)value, output) 
            : WireFormatLite::WriteInt64(field->number(), (int64)value, output);
        break;
    case FieldDescriptor::TYPE_UINT64         :  // uint64, varint on the wire.
        field->is_packed() ? WireFormatLite::WriteUInt64NoTag((uint64)value, output) 
            : WireFormatLite::WriteUInt64(field->number(), (uint64)value, output);
        break;
    case FieldDescriptor::TYPE_INT32          :  // int32, varint on the wire.  Negative numbers
        field->is_packed() ? WireFormatLite::WriteInt32NoTag((int32)value, output) 
            : WireFormatLite::WriteInt32(field->number(), (int32)value, output);
        break;
    case FieldDescriptor::TYPE_UINT32         :  // uint32, varint on the wire
        field->is_packed() ? WireFormatLite::WriteUInt32NoTag((uint32)value, output) 
            : WireFormatLite::WriteUInt32(field->number(), (uint32)value, output);
        break;
    case FieldDescriptor::TYPE_FIXED64        :  // uint64, exactly eight bytes on the wire.
        field->is_packed() ? WireFormatLite::WriteFixed64NoTag((uint64)value, output) 
            : WireFormatLite::WriteFixed64(field->number(), (uint64)value, output);
        break;
    case FieldDescriptor::TYPE_FIXED32        :  // uint32, exactly four bytes on the wire.
        field->is_packed() ? WireFormatLite::WriteFixed32NoTag((uint32)value, output) 
            : WireFormatLite::WriteFixed32(field->number(), (uint32)value, output);
        break;
    case FieldDescriptor::TYPE_SFIXED32       :  // int32, exactly four bytes on the wire
        field->is_packed() ? WireFormatLite::WriteSFixed32NoTag((int32)value, output) 
            : WireFormatLite::WriteSFixed32(field->number(), (int32)value, output);
        break;
    case FieldDescriptor::TYPE_SFIXED64       :  // int64, exactly eight bytes on the wire
        field->is_packed() ? WireFormatLite::WriteSFixed64NoTag((int64)value, output) 
            : WireFormatLite::WriteSFixed64(field->number(), (int64)value, output);
        break;
    case FieldDescriptor::TYPE_SINT32         :  // int32, ZigZag-encoded varint on the wire
        field->is_packed() ? WireFormatLite::WriteSInt32NoTag((int32)value, output) 
            : WireFormatLite::WriteSInt32(field->number(), (int32)value, output);
        break;
    case FieldDescriptor::TYPE_SINT64         :  // int64, ZigZag-encoded varint on the wire
        field->is_packed() ? WireFormatLite::WriteSInt64NoTag((int64)value, output) 
            : WireFormatLite::WriteSInt64(field->number(), (int64)value, output);
        break;
    case FieldDescriptor::TYPE_ENUM           :  // Enum, varint on the wire
        field->is_packed() ? WireFormatLite::WriteEnumNoTag((int)value, output) 
            : WireFormatLite::WriteEnum(field->number(), (int)value, output);
        break;
    default:
        return false;
    }
    return true;
}

bool EncodeBoolean(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index)
{
    bool value = lua_toboolean(L, index) != 0;
    field->is_packed() ? WireFormatLite::WriteBoolNoTag(value, output) 
        : WireFormatLite::WriteBool(field->number(), value, output);
    return true;
}

bool EncodeString(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index)
{
    PROTO_ASSERT(!field->is_packed()); // see protobuf : "[packed = true] can only be specified for repeated primitive fields."
    size_t length = 0;
    const char* bytes = lua_tolstring(L, index, &length);
    WireFormatLite::WriteString(field->number(), string(bytes, length), output);
    return true;
}

bool EncodeMessage(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index)
{
    PROTO_ASSERT(!field->is_packed()); // see protobuf : "[packed = true] can only be specified for repeated primitive fields."
    PROTO_ASSERT(lua_istable(L, index));
    string type_name = field->message_type()->full_name();
    const Descriptor* message = g_descriptor_pool->FindMessageTypeByName(type_name);
    PROTO_ASSERT(message);

    WireFormatLite::WriteTag(field->number(), WireFormatLite::WIRETYPE_LENGTH_DELIMITED, output);
    const char* position = SkipLength(output);
    for (int i = 0; i < message->field_count(); i++)
    {
        const FieldDescriptor* field = message->field(i);
        lua_getfield(L, index, field->name().c_str());
        PROTO_DO(EncodeField(field, output, L, index + 1));
        lua_remove(L, index + 1);
    }
    PROTO_DO(FillLength(output, position));
    return true;
}

bool EncodeTable(const FieldDescriptor* field, CodedOutputStream* output, lua_State* L, int index)
{
    PROTO_ASSERT(!field->is_packed()); // see protobuf : "[packed = true] can only be specified for repeated primitive fields."
    PROTO_ASSERT(lua_istable(L, index));
    string type_name = field->message_type()->full_name();
    const Descriptor* message = g_descriptor_pool->FindMessageTypeByName(type_name);
    PROTO_ASSERT(message);

    lua_pushnil(L);
    while (lua_next(L, index))
    {
        WireFormatLite::WriteTag(field->number(), WireFormatLite::WIRETYPE_LENGTH_DELIMITED, output);
        const char* position = SkipLength(output);
        const FieldDescriptor* key = message->field(0);
        PROTO_DO(EncodeField(key, output, L, index + 1));
        const FieldDescriptor* value = message->field(1);
        PROTO_DO(EncodeField(value, output, L, index + 2));
        lua_pop(L, 1);
        PROTO_DO(FillLength(output, position));
    }
    return true;
}

bool ProtoEncode(const char* proto, lua_State* L, int index, char* output, size_t* size)
{
    PROTO_ASSERT(index > 0);
    const Descriptor* message = g_descriptor_pool->FindMessageTypeByName(proto);
    PROTO_ASSERT(message);

    ArrayOutputStream buffer((void*)output, (int)*size);
    CodedOutputStream stream(&buffer);

    std::vector<const FieldDescriptor*> fields = SortFieldsByNumber(message);
    for (unsigned int i = 0; i < fields.size(); i++)
    {
        const FieldDescriptor* field = fields[i];
        lua_getfield(L, index, field->name().c_str());
        PROTO_DO(EncodeField(field, &stream, L, index + 1));
        lua_remove(L, index + 1);
    }

    *size = stream.ByteCount();
    return true;
}

bool ProtoPack(const char* proto, lua_State* L, int start, int end, char* output, size_t* size)
{
    PROTO_ASSERT(start > 0);
    PROTO_ASSERT(end >= start);
    const Descriptor* message = g_descriptor_pool->FindMessageTypeByName(proto);
    PROTO_ASSERT(message);

    ArrayOutputStream buffer((void*)output, (int)*size);
    CodedOutputStream stream(&buffer);
    
    std::vector<const FieldDescriptor*> fields = SortFieldsByNumber(message);
    for (unsigned int i = 0; i < fields.size(); i++)
    {
        const FieldDescriptor* field = fields[i];
        PROTO_DO(EncodeField(field, &stream, L, start + i));
    }

    *size = stream.ByteCount();
    return true;
}