require "protolua"
proto.parse("person.proto")

local person = {
    name = "jinjiazh",
    id = 10001,
    email = "jinjiazh@qq.com",
    phones = {
        {number = "1818864xxxx", type = PhoneType.HOME},
        {number = "1868200xxxx", type = PhoneType.WORK},
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

require "strings"
print("data = "..b2string(data))
print("clone = "..t2string(clone))
print("name = "..name)
print("id = "..id)
print("email = "..email)
print("phones = "..t2string(phones))
print("scores = "..t2string(scores))