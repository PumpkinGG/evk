#include <sstream>
#include <poll.h>

#include "evk/inner_pre.h"
#include "evk/channel.h"
#include "evk/event_loop.h"

namespace evk {
const int Channel::kNone = 0;
const int Channel::kReadable = POLLIN | POLLPRI;
const int Channel::kWritable = POLLOUT;

Channel::Channel(EventLoop* loop, int fd) 
    : loop_(loop), 
      fd_(fd), 
      events_(0), 
      revents_(0), 
      index_(-1),
      event_handling_(false),
      added_to_loop_(false) {
    DLOG_TRACE << "fd = " << fd;
}

Channel::~Channel() {
    assert(!event_handling_);
    assert(!added_to_loop_);
}

void Channel::Remove() {
    assert(IsNoneEvent());
    added_to_loop_ = false;
    loop_->RemoveChannel(this);
}

void Channel::Update() {
    added_to_loop_ = true;
    loop_->UpdateChannel(this);
}

void Channel::HandleEvent() {
    event_handling_ = true;
    if (revents_ & POLLNVAL) {
        DLOG_WARN << "Channel::HandleEvent() POLLNVAL";
    }
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        if (close_fn_) close_fn_();
    }
    if (revents_ & (POLLERR | POLLNVAL)) {
        if (error_fn_) error_fn_();
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (read_fn_) read_fn_();
    }
    if (revents_ & POLLOUT) {
        if (write_fn_) write_fn_();
    }
    event_handling_ = false;
}

void Channel::EnableReadEvent() {
    events_ |= kReadable;
    Update();
}
void Channel::EnableWriteEvent() {
    events_ |= kWritable;
    Update();
}
void Channel::DisableReadEvent() {
    events_ &= ~kReadable;
    Update();
}
void Channel::DisableWriteEvent() {
    events_ &= ~kWritable;
    Update();
}
void Channel::DisableAllEvent() {
    events_ = kNone;
    Update();
}

} // namespace evk
