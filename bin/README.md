Protocol Buffers for lua
===================================================

https://github.com/jinjiazhang/protolua/


## Windows
**lua.exe:** compile by vs2008<br>
**protolua.dll:** compile with 'RelWithDebInfo' by vs2008

## Linux
**lua:** cd lua-5.3.4 && make linux MYCFLAGS=-fPIC<br>
**protolua.so:** cd protolua && make

# RunTest
**Windows:** lua.exe test.lua<br>
**Linux:** ./lua test.lua<br>
**person.proto:** proto file of person<br>
**serpent.lua:** dump table helper<br>
**test.lua:** test lua code