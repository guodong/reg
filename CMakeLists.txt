cmake_minimum_required (VERSION 2.8)
include(CheckCXXCompilerFlag)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O0 -Wall -g2 -ggdb")
project (reg)

include_directories( ${PROJECT_SOURCE_DIR}/src/include )

aux_source_directory(./src DIR_SRCS)

link_libraries(curl yaml-cpp uuid)

add_executable(reg ${DIR_SRCS})

install(TARGETS reg DESTINATION bin)
