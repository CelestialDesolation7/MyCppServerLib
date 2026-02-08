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
message(STATUS "C++ 标准: C++${CMAKE_CXX_STANDARD}")
# 开启所有警告
add_compile_options(-Wall -Wextra)



# =========================================================
# 目标配置
# =========================================================
include_directories(src)
add_executable(server src/server.cpp src/util.cpp)
add_executable(client src/client.cpp src/util.cpp)