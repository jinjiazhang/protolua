Protocol Buffers for lua
===================================================

https://github.com/jinjiazhang/protolua/

## Dependencies
**lua-5.3.4:** https://www.lua.org/ftp/lua-5.3.4.tar.gz<br>
**protobuf-3.4.1:** https://github.com/google/protobuf/releases/download/v3.4.1/protobuf-cpp-3.4.1.tar.gz<br>

## Windows
**liblua.lib:** compile by vs2008<br>
**libprotobuf.lib:** cmake convert to vs2008, compile with 'RelWithDebInfo'

## Linux
**liblua.a:** cd lua-5.3.4 && make linux MYCFLAGS=-fPIC<br>
**libprotobuf.a:** cd protobuf-3.4.1 && ./configure --with-pic && make