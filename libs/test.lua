require "protolua"
proto.parse("person.proto")

local player = {
	id = 10001,
	name = "jinjiazh",
	email = "jinjiazh@qq.com",
	phones = {
		{number = "1818864xxxx", type = 1},
		{number = "1868200xxxx", type = 2},
	}
}

local data = proto.encode("Person", player)
local copy = proto.decode("Person", data)

local data = proto.pack("Person", player.id, player.name, player.email, player.phones)
local id, name, email, phones = proto.unpack("Person", data)

json = (loadfile "json.lua")()
print(json:encode(copy))
print(id, name, email, json:encode(phones))