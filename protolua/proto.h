#ifndef _JINJIAZHANG_PROTO_H_
#define _JINJIAZHANG_PROTO_H_

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <google/protobuf/descriptor.h>
#include <google/protobuf/wire_format.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

using namespace google::protobuf;
using namespace google::protobuf::io;
using namespace google::protobuf::compiler;
using namespace google::protobuf::internal;

#define PROTO_DO(exp) { if(!(exp)) return false; }
#define PROTO_ASSERT(exp) { if(!(exp)) return false; }

bool ProtoParse(const char* file);
bool ProtoError(const char* format, ...);
bool ProtoEncode(const char* proto, lua_State* L, int index, char* output, size_t* size);
bool ProtoDecode(const char* proto, lua_State* L, const char* input, size_t size);
bool ProtoPack(const char* proto, lua_State* L, int start, int end, char* output, size_t* size);
bool ProtoUnpack(const char* proto, lua_State* L, const char* input, size_t size);

std::vector<const FieldDescriptor*> SortFieldsByNumber(const Descriptor* descriptor);
extern DescriptorPool* g_descriptor_pool;

#endif