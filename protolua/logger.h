#ifndef _JINJIAZHANG_LOGGER_H_
#define _JINJIAZHANG_LOGGER_H_

#define proto_error(format, ...) proto_log(5, format, __VA_ARGS__)
#define proto_warn(format, ...) proto_log(4, format, __VA_ARGS__)
#define proto_info(format, ...) proto_log(3, format, __VA_ARGS__)
#define proto_debug(format, ...) proto_log(2, format, __VA_ARGS__)
#define proto_trace(format, ...) proto_log(1, format, __VA_ARGS__)

#define PROTO_DO(exp) { if(!(exp)) return false; }
#define PROTO_ASSERT(exp) { if(!(exp)) return false; }

void proto_log(int level, const char* format, ...);

#endif