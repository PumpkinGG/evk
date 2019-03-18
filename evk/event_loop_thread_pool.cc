#include "evk/inner_pre.h"
#include "evk/event_loop.h"
#include "evk/event_loop_thread_pool.h"

namespace evk {
EventLoopThreadPool::EventLoopThreadPool(EventLoop* base_loop, const std::string& name)
    : base_loop_(base_loop),
      name_(name),
      num_thread_(0),
      next_(0) {
    DLOG_TRACE << " base_loop = " << base_loop_;
}

EventLoopThreadPool::~EventLoopThreadPool() {
    DLOG_TRACE << "num_thread = " << num_thread_;
    Join();
    threads_.clear();
}

void EventLoopThreadPool::Start(const ThreadInitCallback& cb) {
    status_.store(kStarting);
    DLOG_TRACE << "num_thread = " << num_thread_ << " base_loop = " << base_loop_;

    for (int i = 0; i < num_thread_; i++) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof(buf), "%s-%d", name_.c_str(), i);
        EventLoopThreadPtr t(new EventLoopThread(cb, buf));
        threads_.push_back(t);
        loops_.push_back(t->StartLoop());
    }
    status_.store(kRunning);
    
    if (num_thread_ == 0 && cb) {
        cb(base_loop_);
    }
}

EventLoop* EventLoopThreadPool::GetNextLoop() {
    base_loop_->AssertInLoopThread();
    assert(status_.load() == kRunning);
    EventLoop* loop = base_loop_;
    if (!loops_.empty()) {
        loop = loops_[next_.fetch_add(1)];
        if (static_cast<size_t>(next_) >= loops_.size()) {
            next_.store(0);
        }
    }
    return loop;
}

EventLoop* EventLoopThreadPool::GetNextLoopWithHash(size_t hash_code) {
    base_loop_->AssertInLoopThread();
    assert(status_.load() == kRunning);
    EventLoop* loop = base_loop_;
    if (!loops_.empty()) {
        loop = loops_[hash_code % loops_.size()];
    }
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::GetAllLoops() {
    base_loop_->AssertInLoopThread();
    assert(status_.load() == kRunning);
    if (loops_.empty()) {
        return std::vector<EventLoop*>(1, base_loop_);
    } else {
        return loops_;
    }
}

void EventLoopThreadPool::Stop() {
    DLOG_TRACE;
    status_.store(kStopping);
    if (num_thread_ == 0) {
        status_.store(kStopped);
        return;
    }
    for (auto &t: threads_) {
        t->Stop();
    }
    status_.store(kStopped);
}

void EventLoopThreadPool::Join() {
    DLOG_TRACE << "num_thread = " << num_thread_;
    for (auto &t: threads_) {
        t->Join();
    }
    threads_.clear();
}

} // namespace evk
