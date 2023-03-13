cmake_minimum_required(VERSION 3.16)
project(WordChecker C)

set(CMAKE_C_STANDARD 11)

#set(GCC_COVERAGE_COMPILE_FLAGS "-DEVAL -Wall -Werror -std=gnu11 -O2 -pipe -static -s -lm")
set(GCC_COVERAGE_COMPILE_FLAGS " -Wall -std=gnu11 -pipe -g3 -lm")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -static-libasan")

add_executable(WordChecker main.c)
