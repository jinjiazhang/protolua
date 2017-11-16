Protocol Buffers for lua
===================================================

https://github.com/jinjiazhang/protolua/

## Overview
ProtoLua is a google protocol buffers C library for Lua which implement less than 1000 lines code. Parse proto2 or proto3 file dynamically without code generation, You can also redevelop it to  support RPC like 'Advance Example'.
## Quick Example
person.proto
```C
syntax = "proto2";

message Person {
    required string name = 1;
    required int32 id = 2;
    optional string email = 3;
    
    enum PhoneType {
        MOBILE = 0;
        HOME = 1;
        WORK = 2;
    }
    
    message PhoneNumber {
        required string number = 1;
        optional PhoneType type = 2 [default = MOBILE];
    }
    
    repeated PhoneNumber phones = 4;
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
    }
}

local data = proto.encode("Person", player)
local clone = proto.decode("Person", data)

local data = proto.pack("Person", player.id, player.name, player.email, player.phones)
local id, name, email, phones = proto.unpack("Person", data)
```
## Advance Example
```C
syntax = "proto2";

message OnBuyItemReq {
    required int32 goodsId = 1;
    required int32 goodsNum = 2;
}

message OnBuyItemRsp {
    required int32 retCode = 1;
    required int32 goodsId = 2;
    required int32 goodsNum = 3;
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
