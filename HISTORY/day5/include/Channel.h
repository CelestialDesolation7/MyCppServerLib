#pragma once
#include <cstdint>
#include <sys/epoll.h>
#include <sys/types.h>

class Epoll;

class Channel {
private:
  Epoll *ep;
  int fd;
  uint32_t events;  // 希望监听的事件 (EPOLLIN/EPOLLOUT)
  uint32_t revents; // 目前正在发生的事件 (returned events)
  bool inEpoll;     // 标记当前 Channel 是否已经在 Epoll 树上
public:
  Channel(Epoll *_ep, int _fd);
  ~Channel();

  void enableReading();
  int getFd();
  uint32_t getEvents();
  uint32_t getRevents();

  bool getInEpoll();
  void setInEpoll(bool _in = true);

  void setRevents(uint32_t _rev);
};