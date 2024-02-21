cmake_minimum_required(VERSION 3.20)

project(ducklib-net)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 23)

add_library(${PROJECT_NAME} STATIC
#        ../include/net/Net.h
#        ../include/net/NetClient.h
#        ../include/net/Shared.h
#        ../include/net/Socket.h
        src/Net.cpp
        src/NetClient.cpp
        src/Shared.cpp
        src/Socket.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ../include)