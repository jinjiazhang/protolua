cmake_minimum_required(VERSION 3.1)
project(protolua)

add_definitions(-std=c++11)
aux_source_directory(. DIR_SRCS)
file(GLOB_RECURSE DIR_INCS *.h *.hpp)
source_group("Include Files" FILES ${DIR_INCS}) 

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -undefined dynamic_lookup")
endif()

IF (CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "Darwin")
    add_library(${PROJECT_NAME} SHARED ${DIR_SRCS} ${DIR_INCS})
    target_link_libraries(${PROJECT_NAME} libprotobuf)
ELSE ()
    add_library(${PROJECT_NAME} SHARED ${DIR_SRCS} ${DIR_INCS})
    target_link_libraries(${PROJECT_NAME} lua-${LUA_VERSION} libprotobuf)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "Darwin")


include_directories(../)
include_directories(../thirdparty/lua-${LUA_VERSION}/src)
include_directories(../thirdparty/protobuf/src)
add_dependencies(${PROJECT_NAME} lua-${LUA_VERSION} libprotobuf)
