#pragma once
#include "Channel.h"
#include <sys/epoll.h>
#include <vector>

class Epoll {
  private:
    int epfd;
    struct epoll_event *events;

  public:
    Epoll();
    ~Epoll();
    void updateChannel(Channel *channel);

    // 等待时间发生，返回活跃 Channel 表,而不是 epoll_event
    std::vector<Channel *> poll(int timeout = -1);
};