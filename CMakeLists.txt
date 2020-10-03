 cmake_minimum_required(VERSION 2.8.12)
 project(Necronia)

 add_definitions("-std=c++2a")

 include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
 conan_basic_setup()

 find_package(Protobuf REQUIRED)
 protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS sensor.proto)

 add_executable(Necronia main.cpp ${PROTO_SRCS} ${PROTO_HDRS})
 target_link_libraries(Necronia ${CONAN_LIBS} )
 target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR})
