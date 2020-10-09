cmake_minimum_required(VERSION 3.16)



project(Server CXX)
add_definitions("-std=c++2a")

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/develop/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake"
            TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(CONANFILE conanfile.txt
        BASIC_SETUP BUILD missing KEEP_RPATHS)

enable_testing()

#find_package(GTest REQUIRED)

add_subdirectory(src)
add_subdirectory(tests)
# Parse all protobuff files


add_executable(Server main.cpp)
message("Linking conan libraries: ${CONAN_LIBS} ")
target_link_libraries(Server ServerLib ${CONAN_LIBS} )

# target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/src )
