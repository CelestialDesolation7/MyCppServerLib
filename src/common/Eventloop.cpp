#include "EventLoop.h"
#include "Channel.h"
#include "Epoll.h"
#include "util.h"
#include <functional>
#include <mutex>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

Eventloop::Eventloop() : ep_(nullptr), quit_(false) {
    ep_ = new Epoll(); // 这个是我们自定义的epoll，不是OS给的
    evtfd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    ErrIf(evtfd_ == -1, "eventfd create error in Eventloop.");
    evtChannel_ = new Channel(this, evtfd_);
    evtChannel_->setReadCallback(std::bind(&Eventloop::handleWakeup, this));
    evtChannel_->enableReading();
}

Eventloop::~Eventloop() {
    delete ep_;
    delete evtChannel_;
}

void Eventloop::handleWakeup() {
    int i = 1;
    (void)read(evtfd_, &i, sizeof(i));
}

void Eventloop::wakeup() {
    int i = 1;
    (void)write(evtfd_, &i, sizeof(i));
}

void Eventloop::loop() {
    while (!quit_) {
        std::vector<Channel *> channels;
        channels = ep_->poll(); // 这里会返回
        for (auto it = channels.begin(); it != channels.end(); ++it) {
            (*it)->handleEvent();
        }
        doPendingFunctors();
    }
}

void Eventloop::queueInLoop(std::function<void()> func) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(func);
    }
    wakeup();
}

void Eventloop::doPendingFunctors() {
    std::vector<std::function<void()>> functors;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for (const auto &func : functors) {
        func();
    }
}

void Eventloop::updateChannel(Channel *ch) {
    ep_->updateChannel(ch);
    // 以后要更新channel走eventloop中转
}

void Eventloop::setQuit() { this->quit_ = true; }