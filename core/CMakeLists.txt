cmake_minimum_required(VERSION 3.20)

project(ducklib-core)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 23)

add_library(${PROJECT_NAME} STATIC
        src/Logger.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ../include)
set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        OUTPUT_NAME ${PROJECT_NAME}
        SUFFIX ".lib")