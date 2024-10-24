cmake_minimum_required(VERSION 3.10) 
project(LioNet)

# 设置编译器标志
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -O3 -g -Wall -Wno-deprecated -Werror -Wno-unused-function")

# 生成 compile_commands.json 文件
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR})

# 源文件
set(LIB_SRC
    LioNet/log.cc
    LioNet/util.cc
)

# 添加库
add_library(lionet SHARED ${LIB_SRC})

# 添加可执行文件
add_executable(test_log tests/test_log.cc)
add_dependencies(test_log lionet)
target_link_libraries(test_log lionet)


