#include <sys/socket.h> // 操作系统网络库
#include <arpa/inet.h>  // IP地址翻译库
#include <cstring>
#include <cstdio>
#include <unistd.h>

int main() {
    // 创建套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
}