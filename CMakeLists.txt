cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(MultithreadWebServer)

file(GLOB CPP_SOURCES "Source/*.cc")

add_executable(${PROJECT_NAME} ${CPP_SOURCES})
