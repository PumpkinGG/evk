#include "evk/inner_pre.h"
#include "evk/invoke_timer.h"

namespace evk {
std::atomic<int> InvokeTimer::num_created_(0);

// atomic<T>::fetch_add(int) atomic adds int() and return old value,
// so need to plus 1 to initialize sequence_ with new value.
InvokeTimer::InvokeTimer(const TimerCallback& cb, 
                         Timestamp when, double interval)
    : expiration_(when), callback_(cb),
      interval_(interval), repeat_(interval > 0.0),
      sequence_(num_created_.fetch_add(1) + 1) {
    DLOG_TRACE;
}

InvokeTimer::InvokeTimer(TimerCallback&& cb, 
                         Timestamp when, double interval)
    : expiration_(when), callback_(std::move(cb)),
      interval_(interval), repeat_(interval > 0.0),
      sequence_(num_created_.fetch_add(1) + 1) {
    DLOG_TRACE;      
}
    
InvokeTimer::~InvokeTimer() {
    DLOG_TRACE;
}

void InvokeTimer::OnTimerTriggered() {
    DLOG_TRACE << "timer " << sequence_ << " expired";
    callback_();
}

void InvokeTimer::Restart(Timestamp now) {
    DLOG_TRACE << "timer " << sequence_ << " restart";
    if (repeat_) {
        expiration_ = now + interval_;
    } else {
        expiration_ = Timestamp::Invalid();
    }
}

} // namespace evk
