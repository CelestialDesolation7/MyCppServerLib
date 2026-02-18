#pragma once
#include "Channel.h"
#include "Macros.h"
#include <sys/epoll.h>
#include <vector>

class Epoll {
    DISALLOW_COPY_AND_MOVE(Epoll)
  private:
    int epfd_;
    struct epoll_event *events_;

  public:
    Epoll();
    ~Epoll();
    void updateChannel(Channel *channel);
    void deleteChannel(Channel *channel);

    // 等待时间发生，返回活跃 Channel 表,而不是 epoll_event
    std::vector<Channel *> poll(int timeout = -1);
};