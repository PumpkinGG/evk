#pragma once

#include <atomic>

#include "evk/inner_pre.h"
#include "evk/timestamp.h"
#include "evk/callbacks.h"

namespace evk {
// 
// InvokeTimer for timer event, 
// InvokeTimerID for managing InvokeTimer's lifetime.
// Internal class for timer event.
//
class InvokeTimer {
public:
    InvokeTimer(const TimerCallback& cb, Timestamp when, double interval);
    InvokeTimer(TimerCallback&& cb, Timestamp when, double interval);

    ~InvokeTimer();
    // on time run callback    
    void OnTimerTriggered(); 
    void Restart(Timestamp now);

public:
    Timestamp Expiration() const {
        return expiration_;
    }
    bool Repeat() const {
        return repeat_;
    }
    int Sequence() const {
        return sequence_;
    }

    static int NumCreated() {
        return num_created_.load();
    }

private:
    Timestamp expiration_;
    const TimerCallback callback_;
    const Duration interval_;
    const bool repeat_;
    const int sequence_; // int64_t in muduo
    
    static std::atomic<int> num_created_;

};

//
// InvokeTimerID manages InvokeTimer's lifetime.
// Hide the detail of InvokeTimer to user.
//
class InvokeTimerID {
public:
    InvokeTimerID()
        : timer_(nullptr), sequence_(0) {}
    InvokeTimerID(InvokeTimer* timer, int seq)
        : timer_(timer), sequence_(seq) {}

    friend class TimerQueue;

private:
    InvokeTimer* timer_;
    int sequence_;

};

} // namespace evk
