cmake_minimum_required(VERSION 3.1)
project(thirdparty)

option (protobuf_BUILD_TESTS "protobuf build tests" OFF)
option (protobuf_BUILD_PROTOC_BINARIES "protobuf build binaries" OFF)

add_subdirectory(lua-5.3.5)
add_subdirectory(protobuf/cmake)