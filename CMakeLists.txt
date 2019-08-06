cmake_minimum_required (VERSION 2.6)
project (Tinyjson_test C)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall")
endif()

add_library(Tyjson TyJson.c)
add_executable(json_test test.c)
target_link_libraries(json_test Tyjson)