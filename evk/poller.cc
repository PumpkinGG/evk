#include <poll.h>

#include "evk/inner_pre.h"
#include "evk/poller.h"
#include "evk/channel.h"

namespace evk {
Poller::Poller(EventLoop* loop)
    : owner_loop_(loop) {}

Poller::~Poller() {}

Timestamp Poller::poll(int timeout_ms, ChannelList* active_channels) {
    int num_events = ::poll(&(*pollfd_list_.begin()), pollfd_list_.size(), timeout_ms);
    Timestamp now(Timestamp::Now());
    
    if (num_events > 0) {
        DLOG_TRACE << num_events << " events happended";
        // using poll, need to iterate pollfd_list_ to find active events.
        FillActiveChannels(num_events, active_channels);
    } else if (num_events == 0) {
        DLOG_TRACE << " nothing happended";
    } else {
        LOG_ERROR << "Poller::poll()";
    }

    return now;
}

void Poller::FillActiveChannels(int num_events, ChannelList* active_channels) {
    for (const auto &pfd: pollfd_list_) {
        if (pfd.revents > 0) {
            --num_events;

            auto pch = channel_map_.find(pfd.fd);
            assert(pch != channel_map_.end());
            auto channel = pch->second;
            assert(channel->fd() == pfd.fd);
            channel->SetRevents(pfd.revents);
            active_channels->push_back(channel);

            if (num_events <= 0)
                break;
        }
    }
}

void Poller::UpdateChannel(Channel* channel) {
    AssertInLoopThread();
    DLOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
    if (channel->index() < 0) {
        // index is the index in PollfdList
        // index = -1 when channel constructs
        assert(channel_map_.find(channel->fd()) == channel_map_.end());
        
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfd_list_.push_back(pfd);

        int idx = static_cast<int>(pollfd_list_.size()) - 1;
        channel->SetIndex(idx);
        channel_map_[pfd.fd] = channel;
    } else {
        // update existing one
        assert(channel_map_.find(channel->fd()) != channel_map_.end());
        assert(channel_map_[channel->fd()] == channel);

        int idx = channel->index();
        assert(idx >= 0 && idx < static_cast<int>(channel_map_.size()));

        struct pollfd& pfd = pollfd_list_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd()-1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        // if do not care the events in a channel, set pfd.fd to -1.
        // then poll(2) will not listen this fd
        if (channel->IsNoneEvent()) {
            pfd.fd = -channel->fd()-1;
        }
    }
}

bool Poller::HasChannel(Channel* channel) {
    AssertInLoopThread();
    auto it = channel_map_.find(channel->fd());
    return it != channel_map_.end() && it->second == channel;
}

void Poller::RemoveChannel(Channel* channel) {
    AssertInLoopThread();
    DLOG_TRACE;
    assert(channel_map_.find(channel->fd()) != channel_map_.end());
    assert(channel_map_[channel->fd()] == channel);
    assert(channel->IsNoneEvent());

    int idx = channel->index();
    assert(idx >= 0 && idx < static_cast<int>(pollfd_list_.size()));
    const struct pollfd& pfd = pollfd_list_[idx]; (void)pfd;
    
    // erase channel in channel_map_
    size_t n = channel_map_.erase(channel->fd());
    assert(n == 1); (void)n;
    // erase pollfd in pollfd_list_
    if (idx == static_cast<int>(pollfd_list_.size()) - 1) {
        pollfd_list_.pop_back();
    } else {
        int fdAtEnd = pollfd_list_.back().fd;
        iter_swap(pollfd_list_.begin()+idx, pollfd_list_.end()-1);
        if (fdAtEnd < 0) {
            fdAtEnd = -fdAtEnd-1;
        }
        channel_map_[fdAtEnd]->SetIndex(idx);
        pollfd_list_.pop_back();
    }
}

} // namespace evk
