#pragma once

#include <atomic>
#include <thread>
#include <mutex>
#include <vector>

#include <boost/any.hpp>

#include "evk/inner_pre.h"

namespace evk {
class Channel;

class EventLoop: public noncopyable {
public:
    EventLoop();
    ~EventLoop();

    // Run the IO Event driving loop forever
    void Run();
    
    // Stop the event loop
    void Stop();

    // Getter and Setter
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
    std::atomic<bool> is_running_;
    const std::thread::id tid_;
 
};

}
