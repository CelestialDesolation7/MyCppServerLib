#pragma once
#include <cstdint>
#include <sys/epoll.h>
#include <vector>

class Epoll {
private:
  int epfd;
  struct epoll_event *events;

public:
  Epoll();
  ~Epoll();
  // 将某个 fd 添加到 epoll 监控
  void addFd(int fd, uint32_t op);

  // 等待时间发生，返回活跃事件表
  std::vector<epoll_event> poll(int timeout = -1);
};