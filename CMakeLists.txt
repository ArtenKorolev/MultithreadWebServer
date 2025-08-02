cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(MultithreadWebServer)

file(GLOB CPP_SOURCES
        "Source/*.cc"
        "Source/Socket/*.cc"
        "Source/Server/*.cc"
        "Source/ThreadPool/*.cc"
        "Source/Http/*.cc"
        "Source/Utils/*.cc"
)

add_executable(${PROJECT_NAME} ${CPP_SOURCES})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -g)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE -O2 -Wall -fno-rtti)
endif ()

target_include_directories(${PROJECT_NAME} PRIVATE
        Source/Socket
        Source/Server
        Source/ThreadPool
        Source/Http
        Source/Utils
)
