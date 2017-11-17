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

local data = proto.pack("Person", player.name, player.id, player.email, player.phones)
local name, id, email, phones = proto.unpack("Person", data)

json = (loadfile "json.lua")()
print(json:encode(clone))
print(name, id, email, json:encode(phones))