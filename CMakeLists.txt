cmake_minimum_required(VERSION 3.21)

# 生成 compile_commands.json，供 VS Code 的 C/C++ 插件和 clang-tidy 使用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 项目信息
project(MyCppServerLib VERSION 1.0.0 LANGUAGES CXX)

# C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项：开启常用警告
add_compile_options(-Wall -Wextra)
if(UNIX AND NOT APPLE)
    add_compile_definitions(OS_LINUX)
elseif(APPLE)
    add_compile_definitions(OS_MACOS)
endif()

# 头文件路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/include)

# 自动查找所有源文件
file(GLOB_RECURSE SOURCES "src/common/*.cpp" "src/*.cpp")
# 排除 main 函数所在的 server.cpp 和 client.cpp，因为我们要分别编译它们
list(FILTER SOURCES EXCLUDE REGEX "src/server.cpp")
list(FILTER SOURCES EXCLUDE REGEX "src/client.cpp")
# 同样排除测试文件
list(FILTER SOURCES EXCLUDE REGEX "src/test/.*")

# 1. 编译核心库 (可选，也可以像之前一样直接编译进可执行文件，这里做成静态库更规范)
add_library(NetLib STATIC ${SOURCES})

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