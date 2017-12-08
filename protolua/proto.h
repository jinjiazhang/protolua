#ifndef _JINJIAZHANG_PROTO_H_
#define _JINJIAZHANG_PROTO_H_

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "logger.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/importer.h>

using namespace google::protobuf;
using namespace google::protobuf::io;
using namespace google::protobuf::compiler;
using namespace google::protobuf::internal;

#define PROTO_DO(exp) { if(!(exp)) return false; }
#define PROTO_ASSERT(exp) { if(!(exp)) return false; }

bool ProtoParse(const char* file);
bool ProtoEncode(const char* proto, lua_State* L, int index, char* output, size_t* size);
bool ProtoDecode(const char* proto, lua_State* L, const char* input, size_t size);
bool ProtoPack(const char* proto, lua_State* L, int start, int end, char* output, size_t* size);
bool ProtoUnpack(const char* proto, lua_State* L, const char* input, size_t size);

std::vector<const FieldDescriptor*> SortFieldsByNumber(const Descriptor* descriptor);
extern Importer g_importer;
extern DynamicMessageFactory g_factory;

#endif