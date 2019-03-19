#pragma once

#include <atomic>
#include <thread>
#include <mutex>
#include <vector>

#include <boost/any.hpp>

#include "evk/inner_pre.h"
#include "evk/server_status.h"
#include "evk/callbacks.h"
#include "evk/timestamp.h"
#include "invoke_timer.h"

namespace evk {
class Channel;
class Poller;
class EPoller;
class TimerQueue;

// One EventLoop takes a certain thread.
// One EventLoop ties a certain Poller with numbers of Channels.
// One Channel ties a certain file descriptor.
class EventLoop: public ServerStatus {
public:
    typedef std::function<void()> Functor;

public:
    EventLoop();
    ~EventLoop();

    // Run the IO Event driving loop forever
    void Run();
    
    // Stop the event loop
    void Stop();

// Using to guarantee "one loop per thread".
public:
    bool IsInLoopThread() const {
        return tid_ == std::this_thread::get_id();
    }
    void AssertInLoopThread() {
        if (!IsInLoopThread())
            AbortNotInLoopThread();
    }
    EventLoop* GetEventLoopOfCurrentThread();

// Manipulate timer queue.
public:
    // run callback at a certain time.
    InvokeTimerID RunAt(Timestamp time, TimerCallback cb);
    // run callback after @c delay_s seconds.
    InvokeTimerID RunAfter(double delay_s, TimerCallback cb);
    // run callback every @c interval_s seconds.
    InvokeTimerID RunEvery(double interval_s, TimerCallback cb);
    // cancel the timer.
    void CancelTimer(InvokeTimerID timerid);

    // Runs callback immediately in the loop thread.
    // It wakes up the loop, and run the cb.
    // If in the same loop thread, cb is run within the function.
    // Safe to call from other threads.
    void RunInLoop(Functor cb);
    // Queues callback in the loop thread.
    // Runs after finish polling.
    // Safe to call from other threads.
    void QueueInLoop(Functor cb);

public:
    bool HasChannel(Channel* channel);
    void UpdateChannel(Channel* channel);
    void RemoveChannel(Channel* channel);
    // Write something to wakeup_fd_ to wakeup this loop,
    // if it is pending on poll(2)/epoll(2) or something.
    void Wakeup();

private:
    void AbortNotInLoopThread();
    // Callback of Wakeup(),
    // using to wake up this loop. 
    void HandleRead();
    // Doing tasks.
    void DoPendingFunctors();

private:
    typedef std::vector<Channel*> ChannelList;

    std::atomic<bool> is_calling_pending_functors_;
    // using to wakeup loop.
    int wakeup_fd_;
    // using to manage wakeup_fd_.
    std::unique_ptr<Channel> wakeup_channel_;
    std::mutex mutex_;
    // @GuardedBy mutex_.
    std::vector<Functor> pending_functors_;

    const std::thread::id tid_;
    std::unique_ptr<EPoller> poller_;
    std::unique_ptr<TimerQueue> timer_queue_;
    ChannelList active_channels_;
 
};

} // namespace evk
