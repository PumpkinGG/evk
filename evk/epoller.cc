#include "evk/inner_pre.h"
#include "evk/epoller.h"
#include "evk/channel.h"

#include <poll.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>

namespace evk {
// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
static_assert(EPOLLIN == POLLIN,        "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI,      "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT,      "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP,  "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR,      "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP,      "epoll uses same flag values as poll");

namespace {
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
} // anonymous namespace

EPoller::EPoller(EventLoop* loop)
    : owner_loop_(loop),
      epollfd_(::epoll_create(EPOLL_CLOEXEC)),
      event_list_(kInitEventListSize) {
    if (epollfd_ < 0) {
        LOG_FATAL << "EPoller::Epoller";
    }      
}

EPoller::~EPoller() {
    ::close(epollfd_);
}

Timestamp EPoller::poll(int timeout_ms, ChannelList* active_channels) {
    DLOG_TRACE << "fd total count " << channel_map_.size();
    int numEvents = ::epoll_wait(epollfd_,
                                 &(*event_list_.begin()),
                                 static_cast<int>(event_list_.size()),
                                 timeout_ms);
    int savedErrno = errno;
    Timestamp now(Timestamp::Now());
    
    if (numEvents > 0) {
        DLOG_TRACE << numEvents << " events happended";
        // using poll, need to iterate pollfd_list_ to find active events.
        FillActiveChannels(numEvents, active_channels);
        if (static_cast<size_t>(numEvents) == event_list_.size()) {
            event_list_.resize(event_list_.size()*2);
        }
    } else if (numEvents == 0) {
        DLOG_TRACE << " nothing happended";
    } else {
        if (savedErrno != EINTR) {
            errno = savedErrno;
            LOG_ERROR << "Poller::poll()";
        }
    }

    return now;
}

void EPoller::FillActiveChannels(int num_events, ChannelList* active_channels) {
    assert(static_cast<size_t>(num_events) <= event_list_.size());
    for (int i = 0; i < num_events; i++) {
        Channel* channel = static_cast<Channel*>(event_list_[i].data.ptr);
#ifndef NDEBUG
        int fd = channel->fd();
        auto it = channel_map_.find(fd);
        assert(it != channel_map_.end());
        assert(it->second == channel);
#endif
        channel->SetRevents(event_list_[i].events);
        active_channels->push_back(channel);
    }
}

void EPoller::UpdateChannel(Channel* channel) {
    AssertInLoopThread();
    DLOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
    const int idx = channel->index();
    if (idx == kNew || idx == kDeleted) {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (idx == kNew) {
            assert(channel_map_.find(fd) == channel_map_.end());
            channel_map_[fd] = channel;
        } else {
            assert(channel_map_.find(fd) != channel_map_.end());
            assert(channel_map_[fd] == channel);
        }
        channel->SetIndex(kAdded);
        Update(EPOLL_CTL_ADD, channel);
    } else {
        Update(EPOLL_CTL_MOD, channel);
    }
}
bool EPoller::HasChannel(Channel* channel) {
    AssertInLoopThread();
    auto it = channel_map_.find(channel->fd());
    return it != channel_map_.end() && it->second == channel;
}

void EPoller::RemoveChannel(Channel* channel) {
    AssertInLoopThread();
    int fd = channel->fd();
    DLOG_TRACE << "fd = " << fd;
    
    assert(channel_map_.find(fd) != channel_map_.end());
    assert(channel_map_[fd] = channel);
    assert(channel->IsNoneEvent());
    
    int idx = channel->index();
    assert(idx == kAdded || idx == kDeleted);
    size_t n = channel_map_.erase(fd);
    assert(n == 1); (void)n;

    if (idx == kAdded) {
        Update(EPOLL_CTL_DEL, channel);
    }
    channel->SetIndex(kNew);
}

void EPoller::Update(int op, Channel* channel) {
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    if (::epoll_ctl(epollfd_, op, fd, &event) < 0) {
        if (op == EPOLL_CTL_DEL) {
            LOG_ERROR << "epoll_ctl op = " << OperationToString(op) << " fd = " << fd;
        } else {
            LOG_FATAL << "epoll_ctl op = " << OperationToString(op) << " fd = " << fd;
        }
    }
}

const char* EPoller::OperationToString(int op) {
    switch (op) {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            return "Unknown Operation";
    }
}

} // namespace evk
