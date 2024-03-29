cmake_minimum_required(VERSION 3.23)

set(PROJECT_NAME torrent-client)
project(${PROJECT_NAME} CXX)

add_subdirectory(externals)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall -Wextra")

set(GMOCK_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/externals/googletest/googlemock/include")
set(GTEST_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/externals/googletest/googletest/include")
set(BENCODE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/externals/bencode.hpp/include")
set(CPR_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/externals/cpr/include")
set(JSON_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/externals/json/include")

enable_testing()

add_subdirectory(src)
