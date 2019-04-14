require "protolua"
proto.parse("person.proto")

local player = {
    name = "jinjiazh",
    id = 10001,
    email = "jinjiazh@qq.com",
    phones = {
        {number = "1818864xxxx", type = PhoneType.HOME},
        {number = "1868200xxxx", type = PhoneType.WORK},
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

serpent = require("serpent")
print("clone = "..serpent.block(clone))
print("name = "..name)
print("id = "..id)
print("email = "..email)
print("phones = "..serpent.block(phones))
print("subjects = "..serpent.block(subjects))