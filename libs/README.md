Protocol Buffers for lua
===================================================

https://github.com/jinjiazhang/protolua/

## Windows
**lua.exe; liblua.lib:** compile by vs2008<br>
**libprotobuf.lib:** compile by vs2008<br>
**protolua.dll:** compile by vs2008<br>
**run test:** .\lua.exe test.lua

## Linux
**lua; liblua.a:** cd lua-5.3.4 && make linux MYCFLAGS=-fPIC<br>
**libprotobuf.a:** cd protobuf-3.4.1 && ./configure --with-pic && make<br>
**protolua.so:** cd protolua && make<br>
**run test:** ./lua test.lua