#include "evk/inner_pre.h"
#include "evk/event_loop.h"

namespace evk {
thread_local EventLoop* loop_in_this_thread_ = nullptr;

EventLoop::EventLoop()
    : is_running_(false), tid_(std::this_thread::get_id()) {
    DLOG_TRACE;
    if (loop_in_this_thread_) {
        LOG_FATAL << "Another EventLoop " << loop_in_this_thread_ 
                  << " exists in this thread " << tid_;
    }
    else {
        loop_in_this_thread_ = this;
    }
}

EventLoop::~EventLoop() {
    assert(!is_running_.load());
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
    assert(!is_running_.load());
    AssertInLoopThread();
    is_running_.store(true);

    LOG_INFO << "EventLoop " << this << " stop.";
    is_running_.store(false);
}


}
