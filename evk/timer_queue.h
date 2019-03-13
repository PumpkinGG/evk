#pragma once

#include <set>
#include <vector>
#include <atomic>

#include "evk/timestamp.h"
#include "evk/callbacks.h"
#include "evk/channel.h"


namespace evk {
class EventLoop;
class InvokeTimer;
class InvokeTimerID;

class TimerQueue {
public:
    explicit TimerQueue(EventLoop*);
    ~TimerQueue();

    //
    // Schedules the callback to be run at given time,
    // repeats if @c interval > 0.0.
    //
    InvokeTimerID AddTimer(TimerCallback cb,
                           Timestamp when, double interval);
    void Cancel(InvokeTimerID timerid);

private:
    // ordered by expiration timestamp and pointer
    typedef std::pair<Timestamp, InvokeTimer*> Entry;
    typedef std::set<Entry> TimerSet;
    // ordered by pointer and sequence
    typedef std::pair<InvokeTimer*, int> ActiveTimer;
    typedef std::set<ActiveTimer> ActiveTimerSet;

private:
    void AddTimerInLoop(InvokeTimer* timer);
    void CancelInLoop(InvokeTimerID timerid);
    // called when timerfd run out time
    void HandleRead();
    // move out all expired timers
    std::vector<Entry> GetExpired(Timestamp now);
    // called when done HandleRead()
    void Reset(const std::vector<Entry>& expired, Timestamp now);
    // called by AddTimerInLoop()
    bool Insert(InvokeTimer* timer);
    
private:
    // owner loop
    EventLoop* loop_;
    // timerfd and tied channel used in timer queue
    const int timerfd_;
    Channel timerfd_channel_;
    // timer list sorted by expiration
    TimerSet timers_;

    // used for Cancel()
    // Because expiration is not stable,
    // it is better for upper class to catch InvokeTimerID.
    // When upper class want to cancel timer, 
    // this class will search InvokeTimerID in active_timers_.
    std::atomic<bool> is_calling_expired_timers_;
    ActiveTimerSet active_timers_;
    ActiveTimerSet canceling_timers_;

};

} // namespace evk
