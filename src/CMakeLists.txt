cmake_minimum_required(VERSION 3.17)

#
#include_directories(build)
#include_directories(net)
#
#add_executable(test
#    build/sensor.pb.cc
#    build/sensor.pb.h
#    net/Server.cpp
#    net/Server.h
#    main.cpp)

project(ServerLib CXX)
add_definitions("-std=c++2a")
# Download automatically, you can also just copy the conan.cmake file

# Parse all protobuff files
file(GLOB_RECURSE PROTO_DEFS LIST_DIRECTORIES false *.proto)

find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_DEFS})

add_library(ServerLib OBJECT main.cpp main.h ${PROTO_SRCS} ${PROTO_HDRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/src )