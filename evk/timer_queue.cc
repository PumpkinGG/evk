#include "evk/inner_pre.h"
#include "evk/duration.h"
#include "evk/timestamp.h"

#include "evk/timer_queue.h"
#include "evk/event_loop.h"
#include "evk/invoke_timer.h"

#include <sys/timerfd.h>
#include <unistd.h>

namespace evk {
// wrapper timerfd syscall
int createtimerfd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, 
                                   TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        LOG_FATAL << "Failed in timerfd_create";
    }
    return timerfd;
}

struct timespec howlongtimefromnow(Timestamp when) {
    Duration duration = when - Timestamp::Now();
    int64_t nanoseconds = duration.Nanoseconds();

    if (nanoseconds < 100 * Duration::kMicrosecond) {
        nanoseconds = 100 * Duration::kMicrosecond;
    }

    struct timespec ts;
    ts.tv_sec  = static_cast<time_t>(nanoseconds / Duration::kSecond);
    ts.tv_nsec = static_cast<long>(nanoseconds % Duration::kSecond);
    return ts;
}

void readtimerfd(int timerfd, Timestamp now) {
    uint64_t howlong;
    ssize_t n = ::read(timerfd, &howlong, sizeof(howlong));
    LOG_INFO << "TimerQueue::HandleRead() " << howlong << " at " << now.ToString();
    if (n != sizeof(howlong)) {
        LOG_ERROR << "readtimerfd reads " << n << " bytes instead of 8";
    }
}

void resettimerfd(int timerfd, Timestamp expiration) {
    // wake up loop by timerfd_settime()
    struct itimerspec new_value;
    struct itimerspec old_value;
    memset(&new_value, 0, sizeof(new_value));
    memset(&old_value, 0, sizeof(old_value));
    new_value.it_value = howlongtimefromnow(expiration);
    
    int ret = ::timerfd_settime(timerfd, 0, &new_value, &old_value);
    if (ret) {
        LOG_ERROR << "timerfd_settime()";
    }
}

// 
// TimerQueue class members
//
TimerQueue::TimerQueue(EventLoop* loop) 
    : loop_(loop), 
      timerfd_(createtimerfd()), 
      timerfd_channel_(loop, timerfd_),
      is_calling_expired_timers_(false) {
    DLOG_TRACE;
    timerfd_channel_.SetReadCallback(
            std::bind(&TimerQueue::HandleRead, this));
    timerfd_channel_.EnableReadEvent();
}

TimerQueue::~TimerQueue() {
    timerfd_channel_.DisableAllEvent();
    // timerfd_channel_.Remove();
    ::close(timerfd_);
    // do not remove channel, since we're in EventLoop::dtor();
    for (const Entry& timer: timers_) {
        delete timer.second;
    }
}

InvokeTimerID TimerQueue::AddTimer(TimerCallback cb,
                                    Timestamp when, double interval) {
    InvokeTimer* timer = new InvokeTimer(std::move(cb), when, interval);
    loop_->RunInLoop(
            std::bind(&TimerQueue::AddTimerInLoop, this, timer));
    return InvokeTimerID(timer, timer->Sequence());
}

void TimerQueue::AddTimerInLoop(InvokeTimer* timer) {
    loop_->AssertInLoopThread();
    bool earliest_changed = Insert(timer);
    if (earliest_changed) {
        resettimerfd(timerfd_, timer->Expiration());
    }
}

bool TimerQueue::Insert(InvokeTimer* timer) {
    loop_->AssertInLoopThread();
    assert(timers_.size() == active_timers_.size());
    // flag: whether this timer's expiration is earliest
    bool earliest_changed = false;
    Timestamp when = timer->Expiration();
    auto iter = timers_.begin();
    if (iter == timers_.end() || when < iter->first) {
        earliest_changed = true;
    }
    {
        auto ret = timers_.insert(Entry(when, timer));
        assert(ret.second); (void)ret;
    }
    {
        auto ret = active_timers_.insert(ActiveTimer(timer, timer->Sequence()));
        assert(ret.second); (void)ret;
    }

    assert(timers_.size() == active_timers_.size());
    return earliest_changed;
}

void TimerQueue::Cancel(InvokeTimerID timerid) {
    loop_->RunInLoop(
            std::bind(&TimerQueue::CancelInLoop, this, timerid));
}

void TimerQueue::CancelInLoop(InvokeTimerID timerid) {
    loop_->AssertInLoopThread();
    assert(timers_.size() == active_timers_.size());
    ActiveTimer timer(timerid.timer_, timerid.sequence_);
    auto iter = active_timers_.find(timer);
    if (iter != active_timers_.end()) {
        size_t n = timers_.erase(Entry(iter->first->Expiration(), iter->first));
        assert(n == 1); (void)n;
        delete iter->first;
        active_timers_.erase(iter);
    } else if (is_calling_expired_timers_.load()) {
        // Maybe in HandleRead() calling timers' callbacks will cancel timer.
        // Timers in canceling_timers_ will be delete after HandleRead() in Reset().
        canceling_timers_.insert(timer);
    }
    assert(timers_.size() == active_timers_.size());
}

void TimerQueue::HandleRead() {
    loop_->AssertInLoopThread();
    Timestamp now(Timestamp::Now());
    // handle read event on timerfd_
    readtimerfd(timerfd_, now);
    // check expired timers
    std::vector<Entry> expired = GetExpired(now);

    is_calling_expired_timers_.store(true);
    canceling_timers_.clear();
    // safe to callback outside critical section
    for (const auto &it: expired) {
        it.second->OnTimerTriggered();
    }
    is_calling_expired_timers_.store(false);
    
    Reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::GetExpired(Timestamp now) {
    assert(timers_.size() == active_timers_.size());
    std::vector<Entry> expired;
    // UINTPTR_MAX is the max value of uint type sufficient to hold a pointer.
    Entry sentry = std::make_pair(now, reinterpret_cast<InvokeTimer*>(UINTPTR_MAX));
    // itor points to the iterator of the first timer Not expired.
    auto iter = timers_.lower_bound(sentry);
    assert(iter == timers_.end() || now < iter->first);
    // add expired timer to the back of vector.
    std::copy(timers_.begin(), iter, back_inserter(expired));
    timers_.erase(timers_.begin(), iter);

    for (const auto &it: expired) {
        ActiveTimer timer(it.second, it.second->Sequence());
        size_t n = active_timers_.erase(timer);
        assert(n == 1); (void)n;
    }

    assert(timers_.size() == active_timers_.size());
    return expired;
}

void TimerQueue::Reset(const std::vector<Entry>& expired, Timestamp now) {
    Timestamp next_expire;

    for (const auto &it: expired) {
        ActiveTimer timer(it.second, it.second->Sequence());
        if (it.second->Repeat() 
            && canceling_timers_.find(timer) == canceling_timers_.end()) {
            it.second->Restart(now);
            Insert(it.second);
        } else {
            delete it.second;
        }
    }

    if (!timers_.empty()) {
        next_expire = timers_.begin()->second->Expiration();
    }
    // next_expire will be valid
    if (next_expire.IsValid()) {
        resettimerfd(timerfd_, next_expire);
    }
}

} // namespace evk
