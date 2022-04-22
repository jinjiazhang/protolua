Protocol Buffers for lua
===================================================

https://github.com/jinjiazhang/protolua/

## Overview
ProtoLua is a google protocol buffers C library for Lua which implement less than 1000 lines code. Parse proto2 or proto3 file dynamically without code generation, You can also redevelop it to  support RPC like 'Advance Example'.
## Quick Example
person.proto
```C
syntax = "proto3";

message Person {
    string name = 1;
    int32 id = 2;
    string email = 3;
    
    enum PhoneType {
        MOBILE = 0;
        HOME = 1;
        WORK = 2;
    }
    
    message PhoneNumber {
        string number = 1;
        PhoneType type = 2;
    }
    
    repeated PhoneNumber phones = 4;
    map<string, int32> scores = 5;
}
```
test.lua
```Lua
require "protolua"
proto.parse("person.proto")

local person = {
    name = "jinjiazh",
    id = 10001,
    email = "jinjiazh@qq.com",
    phones = {
        {number = "183****0402", type = PhoneType.HOME},
        {number = "186****9470", type = PhoneType.WORK},
    },
    scores = {
        ["Chinese"] = 82,
        ["Maths"] = 98,
        ["English"] = 88,
    }
}

local data = proto.encode("Person", person)
local clone = proto.decode("Person", data)

local data = proto.pack("Person", person.name, person.id, person.email, person.phones, person.scores)
local name, id, email, phones, scores = proto.unpack("Person", data)
```
## Advance Example
```C
syntax = "proto3";

message OnBuyItemReq {
    int32 goodsId = 1;
    int32 goodsNum = 2;
}

message OnBuyItemRsp {
    int32 retCode = 1;
    int32 goodsId = 2;
    int32 goodsNum = 3;
}
```
```Lua
-- server code
function c2s.OnBuyItemReq( fd, goodsId, goodsNum )
    local player = FindPlayerByFd(fd)
    local retCode = shopMgr.BuyItem(player, goodsId, goodsNum)
    proto.CallClient(fd, "OnBuyItemRsp", retCode, goodsId, goodsNum)
end

-- client code
function s2c.OnBuyItemRsp( retCode, goodsId, goodsNum )
    print(retCode, goodsId, goodsNum)
end

proto.CallServer("OnBuyItemReq", 1021, 10)
```

## Path Mapping

If you want to put `*.proto` files to other directory, you can use path mapping.

### Example:

Directory Tree:
```
C:\my_project\
    proto\
        a.proto
    other_proto\
        b.proto
    run.exe
D:\any_directory\
    c.proto
```

Map path:
```Lua
require "protolua"

-- Added by default, so there is no need to map "./" and "./proto/"
-- proto.map_path("", "./")
-- proto.map_path("", "./proto/")
proto.map_path("", "./other_proto/")
proto.map_path("", "D:\any_directory\")

-- Now, we can parse the proto files
proto.parse("a.proto")
proto.parse("b.proto")
proto.parse("c.proto")
```

## Attention Please
lua51ext.h for int64
```C
inline long long lua_toint64(lua_State *L, int idx)
{
    if (lua_type(L, idx) == LUA_TSTRING)
    {
        return atoll(lua_tostring(L, idx));
    }
    return (long long)lua_tonumber(L, idx);
}

inline void lua_pushint64(lua_State *L, long long value)
{
    // because of #define LUA_NUMBER_FMT "%.14g"
    if (abs(value) > 99999999999999)
    {
        char str[32];
        lua_pushstring(L, _i64toa(value, str, 10));
        return;
    }

    lua_pushnumber(L, (lua_Number)value);
}
```

## Dependencies
lua-5.3.5: https://www.lua.org/ftp/lua-5.3.5.tar.gz<br>
protobuf: https://github.com/google/protobuf<br>

## Contact Me
QQ：164442955

