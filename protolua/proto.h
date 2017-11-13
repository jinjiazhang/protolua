extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <google/protobuf/descriptor.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

using namespace google::protobuf;
using namespace google::protobuf::io;
using namespace google::protobuf::compiler;
using namespace google::protobuf::internal;

enum LogLevel
{
    LOG_LEVEL_ERROR = 300,
    LOG_LEVEL_WARN = 300,
    LOG_LEVEL_DEBUG = 700,
    LOG_LEVEL_TRACE = 800,
};

#define PROTO_DO(exp) { if(!(exp)) return false; }
#define PROTO_ASSERT(exp) { if(!(exp)) return false; }

#define log_error(f, ...) { ProtoPrint(LOG_LEVEL_ERROR, (f), __VA_ARGS__); }
#define log_warn(f, ...)  { ProtoPrint(LOG_LEVEL_ERROR, (f), __VA_ARGS__); }
#define log_debug(f, ...) { ProtoPrint(LOG_LEVEL_ERROR, (f), __VA_ARGS__); }
#define log_trace(f, ...) { ProtoPrint(LOG_LEVEL_ERROR, (f), __VA_ARGS__); }

bool ProtoParse(const char* file);
void ProtoPrint(int level, const char* format, ...);

bool ProtoEncode(const char* proto, lua_State* L, int index, char* output, size_t* size);
bool ProtoPack(const char* proto, lua_State* L, int start, int end, char* output, size_t* size);

int ProtoDecode(const char* proto, lua_State* L, const char* input, size_t size);
int ProtoUnpack(const char* proto, lua_State* L, const char* input, size_t size);

extern DescriptorPool* g_descriptor_pool;