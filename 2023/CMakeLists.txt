cmake_minimum_required(VERSION 3.5)

project(adventofcode2023)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin")

include(FindPkgConfig)
include(CTest)

file(GLOB_RECURSE SRCS "[0-9]*/*.cpp")

foreach(SRC ${SRCS})
    cmake_path(GET SRC PARENT_PATH TGT_DIR)
    file(RELATIVE_PATH TGT_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ${SRC})
    cmake_path(GET TGT_PATH PARENT_PATH TGT_PATH)
    string(REPLACE "/" "_" TGT ${TGT_PATH})
    string(PREPEND TGT "aoc2023_")
    add_executable(${TGT} ${SRC})

    install(TARGETS ${TGT})

    add_test(NAME "${TGT}_test" COMMAND test-riddle test $<TARGET_FILE:${TGT}> ${TGT_DIR})
endforeach()
