cmake_minimum_required(VERSION 3.21)

# 生成 compile_commands.json，供 VS Code 的 C/C++ 插件和 clang-tidy 使用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 项目信息
project(MyCppServerLib VERSION 1.0.0 LANGUAGES CXX)

# C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 头文件路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/include/Poller)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/include/EventLoop)

# 扫描所有公共源文件（排除平台专有目录和不需要打包的文件）
file(GLOB_RECURSE COMMON_SOURCES "src/common/*.cpp")
list(FILTER COMMON_SOURCES EXCLUDE REGEX ".*/Poller/epoll/.*")
list(FILTER COMMON_SOURCES EXCLUDE REGEX ".*/Poller/kqueue/.*")
list(FILTER COMMON_SOURCES EXCLUDE REGEX ".*/EventLoop/epoll/.*")
list(FILTER COMMON_SOURCES EXCLUDE REGEX ".*/EventLoop/kqueue/.*")

# 按平台加入唯一的那个实现文件
set(PLATFORM_SOURCES "")
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    list(APPEND PLATFORM_SOURCES
        "src/common/Poller/epoll/EpollPoller.cpp")
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    list(APPEND PLATFORM_SOURCES
        "src/common/Poller/kqueue/KqueuePoller.cpp")
else()
    message(FATAL_ERROR "Unsupported OS: ${CMAKE_SYSTEM_NAME}")
endif()

# 1. 核心库
add_library(NetLib STATIC ${COMMON_SOURCES} ${PLATFORM_SOURCES})

# 2. Server
add_executable(server src/server.cpp)
target_link_libraries(server NetLib pthread)

# 3. Client
add_executable(client src/client.cpp)
target_link_libraries(client NetLib pthread)

# 4. Tests
add_executable(StressTest src/test/StressTest.cpp)
target_link_libraries(StressTest NetLib pthread)

add_executable(ThreadPoolTest src/test/ThreadPoolTest.cpp)
target_link_libraries(ThreadPoolTest NetLib pthread)