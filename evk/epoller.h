#pragma once

#include <map>
#include <vector>

#include "evk/inner_pre.h"
#include "evk/event_loop.h"
#include "evk/timestamp.h"

struct epoll_event;

namespace evk {
// IO Multiplexing with poll(2).
class EPoller {
public:
    typedef std::vector<Channel*> ChannelList;

public:
    EPoller(EventLoop* loop);
    ~EPoller();

    // Polls the IO events.
    // Must be called in the loop thread.
    Timestamp poll(int timeout_ms, ChannelList* active_channels);

    // Changes the interested IO events.
    // Must be called in the loop thread.
    void UpdateChannel(Channel* channel);
    bool HasChannel(Channel* channel);
    void RemoveChannel(Channel* channel);

    void AssertInLoopThread() {
        owner_loop_->AssertInLoopThread();
    }

private:
    static const int kInitEventListSize = 16;
    static const char* OperationToString(int op);

    void FillActiveChannels(int num_events, ChannelList* active_channels);
    void Update(int operation, Channel* channel);

private:
    typedef std::vector<struct epoll_event> EventList;
    typedef std::map<int, Channel*> ChannelMap;

    EventLoop* owner_loop_;
    int epollfd_;
    EventList event_list_;
    ChannelMap channel_map_;

};

} // namespace evk
