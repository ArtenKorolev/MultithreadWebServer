cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(MultithreadWebServer)

file(GLOB CPP_SOURCES "Source/*.cc" "Source/Socket/*.cc" "Source/Server/*.cc" "Source/ThreadPool/*.cc")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

add_executable(${PROJECT_NAME} ${CPP_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE Source/Socket Source/Server Source/ThreadPool)
