#pragma once

#include <atomic>
#include <thread>
#include <mutex>
#include <vector>

#include <boost/any.hpp>

#include "evk/inner_pre.h"
#include "evk/server_status.h"

namespace evk {
class Channel;
class Poller;

// One EventLoop takes a certain thread.
// One EventLoop ties a certain Poller with numbers of Channels.
// One Channel ties a certain file descriptor.
class EventLoop: public ServerStatus {
public:
    EventLoop();
    ~EventLoop();

    // Run the IO Event driving loop forever
    void Run();
    
    // Stop the event loop
    void Stop();

public:
    bool IsInLoopThread() const {
        return tid_ == std::this_thread::get_id();
    }
    void AssertInLoopThread() {
        if (!IsInLoopThread())
            AbortNotInLoopThread();
    }
    EventLoop* GetEventLoopOfCurrentThread();

public:
    bool HasChannel(Channel* channel);
    void UpdateChannel(Channel* channel);
    void RemoveChannel(Channel* channel);
    void Wakeup();

private:
    void AbortNotInLoopThread();

private:
    typedef std::vector<Channel*> ChannelList;

    const std::thread::id tid_;
    std::unique_ptr<Poller> poller_;
    ChannelList active_channels_;
 
};

}
