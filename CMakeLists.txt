cmake_minimum_required(VERSION 3.21)

# =========================================================
# 项目配置
# =========================================================

# 启用详细的构建输出
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "Verbose build output")
set(CMAKE_MESSAGE_LOG_LEVEL STATUS)

message(STATUS "========================================")
message(STATUS "  MyCppServerLib 构建配置")
message(STATUS "========================================")
message(STATUS "CMake 版本: ${CMAKE_VERSION}")
message(STATUS "系统: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")
message(STATUS "处理器: ${CMAKE_SYSTEM_PROCESSOR}")

# =========================================================
# 项目声明
# =========================================================

project(MyCppServerLib
    VERSION 1.0.0
    DESCRIPTION "A C++ server library for high-performance applications"
    LANGUAGES CXX
)

message(STATUS "项目名称: ${PROJECT_NAME}")
message(STATUS "项目版本: ${PROJECT_VERSION}")

# =========================================================
# 语言标准设置
# =========================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # 生成 compile_commands.json 供 IDE 使用
message(STATUS "C++ 标准: C++${CMAKE_CXX_STANDARD}")
# 开启所有警告
add_compile_options(-Wall -Wextra)

# =========================================================
# 目标配置
# =========================================================
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
# 设置头文件搜索路径为 src/，这样 #include "include/Epoll.h" 才能正确找到
include_directories(${SOURCE_DIR}/include)

# =========================================================
# 【自动化】自动收集公共库源文件
# =========================================================

# 1. 自动收集 common/ 目录下的所有 .cpp 文件
file(GLOB COMMON_SOURCES "${SOURCE_DIR}/common/*.cpp")

# 打印找到的文件，方便调试
message(STATUS "找到的公共源文件: ${COMMON_SOURCES}")

# 2. 编译 server 可执行文件 (只用 server.cpp + 公共库)
add_executable(server 
    ${SOURCE_DIR}/server.cpp
    ${COMMON_SOURCES}
)

# 3. 编译 client 可执行文件 (只用 client.cpp + 公共库)
add_executable(client 
    ${SOURCE_DIR}/client.cpp
    ${COMMON_SOURCES}
)