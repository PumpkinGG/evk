#include "evk/inner_pre.h"
#include "evk/event_loop.h"
#include "evk/epoller.h"
#include "evk/channel.h"
#include "evk/timer_queue.h"
#include "evk/socket_ops.h"

#include <algorithm>

#include <signal.h>
#include <unistd.h>
#include <sys/eventfd.h>

namespace evk {

namespace {
thread_local EventLoop* loop_in_this_thread_ = nullptr;
const int kPollTimeMs = 10000;

// eventfd
int create_eventfd() {
    int evfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evfd < 0) {
        LOG_FATAL << "Failed in eventfd";
    }
    return evfd;
}

// ignore SIGPIPE
const auto IgnoreSIGPIPE = [](){
    ::signal(SIGPIPE, SIG_IGN);
    return 0;
}();

} // namespace 

EventLoop::EventLoop()
    : is_calling_pending_functors_(false),
      wakeup_fd_(create_eventfd()),
      wakeup_channel_(new Channel(this, wakeup_fd_)),
      tid_(std::this_thread::get_id()),
      poller_(new EPoller(this)),
      timer_queue_(new TimerQueue(this)) {
    DLOG_TRACE;
    if (loop_in_this_thread_) {
        LOG_FATAL << "Another EventLoop " << loop_in_this_thread_ 
                  << " exists in this thread " << tid_;
    }
    else {
        loop_in_this_thread_ = this;
    }
    wakeup_channel_->SetReadCallback(
            std::bind(&EventLoop::HandleRead, this));
    wakeup_channel_->EnableReadEvent();
    status_.store(kInitialized);
}

EventLoop::~EventLoop() {
    DLOG_TRACE;
    assert(status_.load() == kStopped);
    wakeup_channel_->DisableAllEvent();
    // wakeup_channel_->Remove();
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

    while (status_.load() == kRunning) {
        active_channels_.clear();
        poller_->poll(kPollTimeMs, &active_channels_);
        for (auto channel: active_channels_) {
            channel->HandleEvent();
        }
        DoPendingFunctors();
    }

    DLOG_TRACE << "EventLoop stopped.";
    status_.store(kStopped);
}

void EventLoop::Stop() {
    status_.store(kStopping);
    DLOG_TRACE << "EventLoop::Stop";
    if (!IsInLoopThread()) {
        Wakeup();
    }
}

InvokeTimerID EventLoop::RunAt(Timestamp time, TimerCallback cb) {
    return timer_queue_->AddTimer(std::move(cb), time, 0.0);
}

InvokeTimerID EventLoop::RunAfter(double delay_s, TimerCallback cb) {
    Timestamp time = Timestamp::Now() + Duration(delay_s);
    return RunAt(time, std::move(cb));
}

InvokeTimerID EventLoop::RunEvery(double interval_s, TimerCallback cb) {
    Timestamp time = Timestamp::Now() + Duration(interval_s);
    return timer_queue_->AddTimer(std::move(cb), time, interval_s);
}

void EventLoop::CancelTimer(InvokeTimerID timerid) {
    timer_queue_->Cancel(timerid);
}

void EventLoop::RunInLoop(Functor cb) {
    if (IsInLoopThread()) {
        cb();
    } else {
        QueueInLoop(std::move(cb));
    }
}

void EventLoop::QueueInLoop(Functor cb) {
    // critical section 
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pending_functors_.push_back(std::move(cb));
    }
    // If called this member function from other thread,
    // or owner thread of this loop is calling pending functors,
    // wakeup this thread again.
    // ** this member function is always called by other thread **
    if (!IsInLoopThread() || is_calling_pending_functors_.load()) {
        Wakeup();
    }
}

void EventLoop::DoPendingFunctors() {
    std::vector<Functor> functors;
    is_calling_pending_functors_.store(true);
    // @GuardedBy mutex_
    // using swap to decrease critical section
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pending_functors_);
    }

    for (auto &functor: functors) {
        functor();
    }

    is_calling_pending_functors_.store(false);
}

void EventLoop::Wakeup() {
    // write something to wakeup_fd_ to wakeup this loop,
    // if it is pending on poll(2)/epoll(2) or something.
    uint64_t one = 1;
    ssize_t n = sock::Write(wakeup_fd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR << "EventLoop::Wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::HandleRead() {
    // callback of Wakeup()
    uint64_t one = 1;
    ssize_t n = sock::Read(wakeup_fd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR << "EventLoop::HandleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::UpdateChannel(Channel* channel) {
    assert(channel->OwnerLoop() == this);
    AssertInLoopThread();
    poller_->UpdateChannel(channel);
}

bool EventLoop::HasChannel(Channel* channel) {
    assert(channel->OwnerLoop() == this);
    AssertInLoopThread();
    return poller_->HasChannel(channel);
}

void EventLoop::RemoveChannel(Channel* channel) {
    assert(channel->OwnerLoop() == this);
    AssertInLoopThread();
    poller_->RemoveChannel(channel);
}

} // namespace evk
