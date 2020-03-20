cmake_minimum_required(VERSION 3.1)
project(lua51)

aux_source_directory(./src DIR_SRCS)
list(REMOVE_ITEM DIR_SRCS ./src/lua.c ./src/luac.c)
add_library(${PROJECT_NAME} ${DIR_SRCS})