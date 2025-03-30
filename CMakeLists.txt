cmake_minimum_required(VERSION 3.0)
project(mprpc)
# 生成debug版本，可以调试
set(CMAKE_BUILD_TYPE "debug")

# 设置项目可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#设置项目库文件输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#设置项目编译头文件搜索路径
find_package(Protobuf REQUIRED)
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)
include_directories(${Protobuf_INCLUDE_DIRS})

# 设置项目库文件搜索路径
link_directories(${PROJECT_SOURCE_DIR}/lib)

# src包含mprpc框架所有的相关代码
add_subdirectory(src)
# example包含了mprpc框架使用的示例代码
add_subdirectory(example)