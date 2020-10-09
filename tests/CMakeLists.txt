cmake_minimum_required(VERSION 3.16)

include(GoogleTest)
link_directories(${CONAN_LIB_DIRS})

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.cpp)

add_executable(ServerTests ${TEST_SOURCES})
target_link_libraries(ServerTests ${CONAN_LIBS} )
target_include_directories(ServerTests PRIVATE ${CMAKE_SOURCE_DIR}/lib)

gtest_add_tests(TARGET ServerTests
        SOURCES ${TEST_SOURCES})