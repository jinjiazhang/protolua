Protocol Buffers for lua
===================================================

https://github.com/jinjiazhang/protolua/

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
