cmake_minimum_required(VERSION 2.8)
project(LioNet)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

set(LIB_SRC
    LioNet/log.cc
)

add_library(lionet SHARED ${LIB_SRC})

add_executable(test tests/test.cc)
add_dependencies(test lionet)
target_link_libraries(test lionet)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)