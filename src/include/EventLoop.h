#pragma once
#include "Channel.h"
#include "Macros.h"
#include <functional>
#include <mutex>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <vector>

class Epoll;
class Channel;

class Eventloop {
    DISALLOW_COPY_AND_MOVE(Eventloop);

  private:
    int evtfd_;
    Epoll *ep_;
    Channel *evtChannel_;
    bool quit_;
    std::vector<std::function<void()>> pendingFunctors_;
    std::mutex mutex_;

    void doPendingFunctors();
    void handleWakeup();

  public:
    Eventloop();
    ~Eventloop();

    void loop();
    void setQuit();
    void updateChannel(Channel *ch);
    void queueInLoop(std::function<void()> func);
    void wakeup();
};