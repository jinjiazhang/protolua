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
    map<int32, string> subjects = 5;
}
```
test.lua
```Lua
require "protolua"
proto.parse("person.proto")

local player = {
    name = "jinjiazh",
    id = 10001,
    email = "jinjiazh@qq.com",
    phones = {
        {number = "1818864xxxx", type = 1},
        {number = "1868200xxxx", type = 2},
    },
    subjects = {
        [101] = "Chinese",
        [102] = "English",
        [103] = "Maths",
    }
}

local data = proto.encode("Person", player)
local clone = proto.decode("Person", data)

local data = proto.pack("Person", player.name, player.id, player.email, player.phones, player.subjects)
local name, id, email, phones, subjects = proto.unpack("Person", data)
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
## Dependencies
lua-5.3.4: https://www.lua.org/ftp/lua-5.3.4.tar.gz<br>
protobuf-3.4.1: https://github.com/google/protobuf/releases/download/v3.4.1/protobuf-cpp-3.4.1.tar.gz<br>
If you want to use other version of the library, may be you need to alter a little code.<br>
You can compile protolua.dll with 'RelWithDebInfo' config in vs2008.
