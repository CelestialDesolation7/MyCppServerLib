#include "Epoll.h"
#include "Channel.h"
#include "util.h"
#include <cerrno>
#include <cstring>
#include <sys/epoll.h>
#include <unistd.h>

#define MAX_EVENTS 1024

Epoll::Epoll() : epfd_(-1), events_(nullptr) {
    epfd_ = epoll_create1(0);
    ErrIf(epfd_ == -1, "epoll create error");
    events_ = new epoll_event[MAX_EVENTS];
    bzero(events_, sizeof(epoll_event) * MAX_EVENTS);
}

Epoll::~Epoll() {
    if (epfd_ != -1) {
        close(epfd_);
        epfd_ = -1;
    }
    delete[] events_;
}

void Epoll::updateChannel(Channel *channel) {
    int fd = channel->getFd();
    struct epoll_event ev;

    ev.data.ptr = channel;
    ev.events = channel->getEvents();

    if (!channel->getInEpoll()) {
        ErrIf(epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
        channel->setInEpoll();
    } else {
        ErrIf(epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll modify error");
    }
}

void Epoll::deleteChannel(Channel *channel) {
    int fd = channel->getFd();
    ErrIf(epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL) == -1, "epoll delete error");
    channel->setInEpoll(false);
}

std::vector<Channel *> Epoll::poll(int timeout) {
    std::vector<Channel *> activeChannels;
    int nfds = epoll_wait(epfd_, events_, MAX_EVENTS, timeout);
    ErrIf(nfds == -1 && errno != EINTR, "epoll wait error");

    for (int i = 0; i < nfds; ++i) {
        Channel *ch = (Channel *)events_[i].data.ptr;
        ch->setRevents(events_[i].events);
        activeChannels.push_back(ch);
    }
    return activeChannels;
}