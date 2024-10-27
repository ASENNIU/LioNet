cmake_minimum_required(VERSION 3.22.1)
project(LioNet VERSION 1.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置编译选项
add_compile_options(-rdynamic -O3 -g -Wall -Wno-deprecated -Werror -Wno-unused-function)

# 查找 yaml-cpp
find_package(yaml-cpp REQUIRED)
find_package(Boost REQUIRED)

# 设置包含目录
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${YAML_CPP_INCLUDE_DIR})
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

# 生成 compile_commands.json 文件
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# 源文件
set(LIB_SRC
    LioNet/log.cc
    LioNet/util.cc
    LioNet/config.cc
    LioNet/env.cc
)

# 添加库
add_library(lionet SHARED ${LIB_SRC})
target_include_directories(lionet PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(lionet PUBLIC yaml-cpp)

# 添加可执行文件
add_executable(test_log tests/test_log.cc)
target_link_libraries(test_log PRIVATE lionet)

add_executable(test_config tests/test_config.cc)
target_link_libraries(test_config PRIVATE lionet yaml-cpp)
