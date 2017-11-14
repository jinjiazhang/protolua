#include "proto.h"

bool DecodeMessage(const Descriptor* message, CodedInputStream* stream, lua_State* L)
{

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