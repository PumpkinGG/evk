#pragma once

#include "evk/inner_pre.h"
#include "evk/noncopyable.h"

namespace evk {
class EventLoop;

// A selectable I/O channel
//
// This class doesn't own file descriptor.
// The file descriptor could be a socket, 
// an eventfd, a timerfd, or a signalfd.
class Channel: public noncopyable {
public:
    typedef std::function<void()> EventCallback;

public:
    Channel(EventLoop* loop, int fd);
    ~Channel();

    void Remove();

public:
    void HandleEvent();
    void SetReadCallback(const EventCallback& cb) {
        read_fn_ = cb;
    }
    void SetWriteCallback(const EventCallback& cb) {
        write_fn_ = cb;
    }
    void SetCloseCallback(const EventCallback& cb) {
        close_fn_ = cb;
    }
    void SetErrorCallback(const EventCallback& cb) {
        error_fn_ = cb;
    }

public:
    int fd() const {
        return fd_;
    }
    int events() const {
        return events_;
    }
    EventLoop* OwnerLoop() const {
        return loop_;
    }

public:
    bool IsReadable() const {
        return (events_ & kReadable) != 0;
    }
    bool IsWritable() const {
        return (events_ & kWritable) != 0;
    }
    bool IsNoneEvent() const {
        return events_ == kNone;
    }

    void EnableReadEvent();
    void EnableWriteEvent();
    void DisableReadEvent();
    void DisableWriteEvent();
    void DisableAllEvent();

    // For Poller
public:
    int index() const {
        return index_;
    }
    void SetIndex(const int idx) {
        index_ = idx;
    }
    void SetRevents(const int revents) {
        revents_ = revents;
    }

private:
    void Update();

private:
    static const int kNone;
    static const int kReadable;
    static const int kWritable;

    EventCallback read_fn_;
    EventCallback write_fn_;
    EventCallback error_fn_;
    EventCallback close_fn_;

    EventLoop* loop_;
    const int fd_;

    int events_;
    int revents_;
    int index_; // used by poller
    
    bool event_handling_;
    bool added_to_loop_; //

};

} // namespace evk
