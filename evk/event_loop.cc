#include "evk/inner_pre.h"
#include "evk/event_loop.h"
#include "evk/poller.h"
#include "evk/channel.h"

namespace evk {
thread_local EventLoop* loop_in_this_thread_ = nullptr;

EventLoop::EventLoop()
    : tid_(std::this_thread::get_id()),
      poller_(new Poller(this)) {
    DLOG_TRACE;
    if (loop_in_this_thread_) {
        LOG_FATAL << "Another EventLoop " << loop_in_this_thread_ 
                  << " exists in this thread " << tid_;
    }
    else {
        loop_in_this_thread_ = this;
    }
    status_.store(kInitialized);
}

EventLoop::~EventLoop() {
    assert(status_.load() == kStopped);
    loop_in_this_thread_ = nullptr;
}

// Get EventLoop of current thread
EventLoop* EventLoop::GetEventLoopOfCurrentThread() {
    return loop_in_this_thread_;
}

// Abort if run in another thread, Ensure one loop per thread
void EventLoop::AbortNotInLoopThread() {
    LOG_FATAL << "EventLoop::AbortNotInLoopThread - EventLoop " << this
              << " was created in thread " << tid_
              << ", current thread is " << std::this_thread::get_id();
}

// Run the event loop
void EventLoop::Run() {
    DLOG_TRACE;
    assert(status_.load() != kRunning);
    AssertInLoopThread();

    status_.store(kStarting);
    // initializing
    status_.store(kRunning);

    while (status_.load() != kStopping) {
        active_channels_.clear();
        poller_->poll(1000, &active_channels_);
        for (auto channel: active_channels_) {
            channel->HandleEvent();
        }
    }

    DLOG_TRACE << "EventLoop stopped.";
    status_.store(kStopped);
}

void EventLoop::Stop() {
    assert(status_.load() == kRunning);
    status_.store(kStopping);
    DLOG_TRACE << "EventLoop::Stop";
}

void EventLoop::UpdateChannel(Channel* channel) {
    assert(channel->OwnerLoop() == this);
    AssertInLoopThread();
    poller_->UpdateChannel(channel);
}

} // namespace evk
