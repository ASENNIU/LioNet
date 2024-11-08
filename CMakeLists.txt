cmake_minimum_required(VERSION 3.22.1)
project(LioNet VERSION 1.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置编译选项
add_compile_options(-rdynamic -O3 -g -Wall -Wno-deprecated -Werror -Wno-unused-function)

if(DEFINED ENV{CONDA_PREFIX})
  set(CMAKE_IGNORE_PATH $ENV{CONDA_PREFIX})
endif()

# 查找 第三方库
find_package(yaml-cpp REQUIRED)
find_package(Boost REQUIRED)
find_package(benchmark REQUIRED)
find_library(RT_LIBRARY rt)

# 设置包含目录
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/LioNet)

include_directories(${YAML_CPP_INCLUDE_DIR})

if(NOT RT_LIBRARY)
  message(FATAL_ERROR "librt not found")
endif()

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
    LioNet/thread.cc
    LioNet/mutex.cc
    LioNet/fiber.cc
    LioNet/scheduler.cc
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

add_executable(test_thread tests/test_thread.cc)
target_link_libraries(test_thread PRIVATE lionet)

add_executable(test_util tests/test_util.cc)
target_link_libraries(test_util PRIVATE lionet)

add_executable(test_fiber tests/test_fiber.cc)
target_link_libraries(test_fiber PRIVATE lionet)

add_executable(test_fiber_bm tests/test_fiber_bm.cc)
target_link_libraries(test_fiber_bm PRIVATE lionet benchmark::benchmark ${RT_LIBRARY})

add_executable(test_scheduler tests/test_scheduler.cc)
target_link_libraries(test_scheduler PRIVATE lionet)

