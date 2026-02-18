#pragma once
#include <functional>
#include <sys/epoll.h>

class Epoll;
class Eventloop;

class Channel {
  private:
    Eventloop *loop;
    int fd;
    uint32_t events;  // 希望监听的事件 (EPOLLIN/EPOLLOUT)
    uint32_t revents; // 目前正在发生的事件 (returned events)
    bool inEpoll;     // 标记当前 Channel 是否已经在 Epoll 树上
    std::function<void()> readCallback;
    std::function<void()> writeCallback;

  public:
    Channel(Eventloop *_ep, int _fd);
    ~Channel();

    void handleEvent(); // 处理事件
    void enableReading();
    void enableWriting();
    void disableWriting();
    bool isWriting();

    int getFd();
    uint32_t getEvents();
    uint32_t getRevents();
    bool getInEpoll();
    void setInEpoll(bool _in = true);
    void setRevents(uint32_t _rev);

    void setReadCallback(std::function<void()> _cb);
    void setWriteCallback(std::function<void()> _cb);
};