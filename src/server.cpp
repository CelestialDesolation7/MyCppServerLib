#include "util.h"
#include <arpa/inet.h> // IP地址翻译库
#include <cerrno>      // 错误码库
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h> // 操作系统网络库
#include <sys/types.h>
#include <unistd.h>

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

// 辅助函数：设置文件描述符为非阻塞模式
void setnonblocking(int fd) {
  int oldoptions = fcntl(fd, F_GETFL);
  int new_option = oldoptions | O_NONBLOCK;
  fcntl(fd, F_SETFL, new_option);
}

int main() {
  // 创建 socket ，这是一个整数句柄，指向一个内核中的结构体，参数：用ipv4 tcp
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  errif(sockfd == -1, "[服务器] socket创建失败");

  // 将上述新申请的 socket 初始化。先构造出本 server 的地址，端口，协议
  struct sockaddr_in server_addr;
  std::memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(
      "127.0.0.1"); // 转化字符串ip到整数形式,处理主机字节的小端序转化为网路字节的大端序
  server_addr.sin_port = htons(8888);

  // 将内核中尚无意义的 socket 结构与我们所制定的端口关联（Bind）
  errif(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
            -1,
        "[服务器] socket绑定失败");

  // 内核将上述 socket 标记为 LISTEN 状态
  // 这意味着内核为其分配了半连接/全连接（TCP）队列
  // 此时客户端已经可以与此 socket 建立TCP连接（或半连接），握手由协议栈处理
  errif(listen(sockfd, SOMAXCONN) == -1, "[服务器] 建立listen失败");
  std::cout << "[服务器] 正在监听 127.0.0.1:8888" << std::endl;

  // IO复用：一个线程（服务器线程，复用者）同时监控多个
  // socket_fd（文件描述符，对应于多个IO事件）
  // 1. 创建 epoll 实例 (参数 0 即可，旧 Linux 需要传大小，现在被忽略)
  // 返回值 epfd 也是一个文件描述符，代表这个 epoll 实例
  int epfd = epoll_create1(0);
  errif(epfd == -1, "[服务器] Epoll 创建错误");
  // 2. 准备两个事件结构体
  // events数组: 用来接收 epoll_wait 返回的“发生了事情的列表”
  // ev: 用来配置“要把哪个 socket 加入监控，监控什么事件”
  struct epoll_event events[MAX_EVENTS];
  struct epoll_event ev;
  // 3. 将 server socket (监听套接字) 加入 epoll
  ev.data.fd = sockfd;
  ev.events = EPOLLIN | EPOLLET;
  setnonblocking(sockfd);
  epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

  while (true) {
    // 获得发生事件的fd个数，通过events数组接受
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
    for (int i = 0; i < nfds; ++i) {
      if (events[i].data.fd ==
          sockfd) { // 发生事件的fd与本服务器的socketfd有关，说明有新连接建立，而不是旧的连接传输数据
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        bzero(&client_addr, sizeof(client_addr));

        // 取出新的fd
        int client_sockfd =
            accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        errif(client_sockfd == -1, "[服务器] socket accept error");
        std::cout << "[服务器] 新的客户端连接，fd: " << client_sockfd
                  << " IP: " << inet_ntoa(client_addr.sin_addr)
                  << " Port: " << ntohs(client_addr.sin_port) << std::endl;

        // 将新的 fd 加入 epoll 红黑树，监控其读事件
        bzero(&ev, sizeof(ev));
        ev.data.fd = client_sockfd;
        ev.events = EPOLLIN | EPOLLET;
        setnonblocking(client_sockfd);
        epoll_ctl(epfd, EPOLL_CTL_ADD, client_sockfd, &ev);
      } else if (events[i].events & EPOLLIN) {
        char buf[READ_BUFFER];
        // 由于我们设置了 EPOLLET (边缘触发)，内核通知我们“有数据”只通知一次。
        // 假如缓冲区有 1000 字节，我们 read 了 500 字节，剩下 500
        // 字节如果不读完， 除非对方再发新数据，否则内核不会再次通知我们。
        // 所以：必须循环读取，直到读空（返回 EAGAIN）
        while (true) {
          bzero(&buf, sizeof(buf));
          ssize_t bytes_read = read(events[i].data.fd, buf, sizeof(buf));
          if (bytes_read > 0) {
            std::cout << "[服务器] 收到客户端 fd " << events[i].data.fd
                      << " 的消息: " << buf << std::endl;
            // 将消息原样返回给客户端
            write(events[i].data.fd, buf, bytes_read);
          } else if (bytes_read == -1 && errno == EINTR) {
            // 被信号中断，继续读取
            std::cout << "[服务器] 读取客户端 fd " << events[i].data.fd
                      << " 时被信号中断，继续读取" << std::endl;
            continue;
          } else if (bytes_read == -1 &&
                     (errno == EAGAIN || errno == EWOULDBLOCK)) {
            // 读完了，缓冲区空了，等待下次通知
            std::cout << "[服务器] 客户端 fd " << events[i].data.fd
                      << " 的消息已读完，等待下次通知" << std::endl;
            break;
          } else if (bytes_read == 0) {
            // 客户端关闭连接
            std::cout << "[服务器] 客户端 fd " << events[i].data.fd
                      << " 已关闭连接" << std::endl;
            close(events[i].data.fd);
            break;
          }
        }
      }
    }
  }
  close(sockfd);
  return 0;
}