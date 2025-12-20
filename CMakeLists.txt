cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(MultithreadWebServer LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 10.2.1
)

FetchContent_MakeAvailable(fmt)

file(GLOB CPP_SOURCES
        "Source/*.cc"
        "Source/Socket/OS/Unix/*.cc"
        "Source/Socket/*.cc"
        "Source/Server/*.cc"
        "Source/ThreadPool/*.cc"
        "Source/Http/*.cc"
        "Source/Utils/*.cc"
        "Source/Signals/*.cc"
        "Source/Config/Ini/*.cc"
)

add_executable(${PROJECT_NAME} ${CPP_SOURCES}
        Source/Config/Ini/IniParser.cc
        Source/Config/Ini/IniParser.h
        Source/Config/Config.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -g)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE -O2 -Wall -fno-rtti)
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

target_include_directories(${PROJECT_NAME} PRIVATE
        Source/Socket/OS/Unix
        Source/Socket
        Source/Server
        Source/ThreadPool
        Source/Http
        Source/Utils
        Source/Signals
        Source/Config
        Source/Config/Ini
)

# =============== TESTS ===============

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
)

FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB TESTS_SRC
        "Tests/*.cc"
        "Source/Http/*.cc"
        "Source/Utils/*.cc"
        "Source/Config/Ini/*.cc"
)

add_executable(tests ${TESTS_SRC})

target_link_libraries(tests gtest gtest_main fmt::fmt)

target_include_directories(tests PRIVATE
        Source/Http
        Source/Utils
        Source/Config/Ini
        Source/Config)

add_test(NAME WebServerTests COMMAND tests)
