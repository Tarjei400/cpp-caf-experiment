cmake_minimum_required(VERSION 3.17)


project(tests CXX)
add_definitions("-std=c++2a")

set(BINARY ${CMAKE_PROJECT_NAME}_tests)

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/develop/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake"
            TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(CONANFILE conanfile.txt
        BASIC_SETUP  BUILD )

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.cpp)
set(SOURCES ${TEST_SOURCES})

add_executable(${BINARY} ${TEST_SOURCES})

add_executable(tests main.cpp)
message("Linking conan libraries: ${CONAN_LIBS} ")

target_link_libraries(tests ${CONAN_LIBS} )
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR})