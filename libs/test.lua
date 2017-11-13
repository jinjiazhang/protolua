require "protolua"
proto.parse("person.proto")

local player = {
	name = "jinjiazh",
	id = 10001,
	email = "jinjiazh@qq.com",
	phones = {
		{number = "1818864xxxx", type = 0},
		{number = "1868200xxxx", type = 1},
	}
}
local data = proto.pack("Person", player)
proto.debug("Person", data)