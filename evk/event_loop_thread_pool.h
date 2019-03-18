#pragma once

#include <atomic>
#include "evk/event_loop_thread.h"

namespace evk {
class EventLoopThreadPool : public ServerStatus {
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;

public:
    EventLoopThreadPool(EventLoop* base_loop, const std::string& name);
    ~EventLoopThreadPool();

    void Start(const ThreadInitCallback& cb = ThreadInitCallback());
    void SetThreadNum(int num_thread) {
        num_thread_ = num_thread;
    }
    void Stop();

    // @brief Join all the working thread. If you forget to call this method,
    // it will be invoked automatically in the destruct method ~EventLoopThreadPool().
    // @note DO NOT call this method from any of the working thread.
    void Join();

public:
    EventLoop* GetNextLoop();
    EventLoop* GetNextLoopWithHash(size_t hash_code);
    std::vector<EventLoop*> GetAllLoops();

    const std::string& Name() const {
        return name_;
    }

private:
    EventLoop* base_loop_;
    std::string name_;
    int num_thread_;
    std::atomic<int> next_;

    typedef std::shared_ptr<EventLoopThread> EventLoopThreadPtr;
    std::vector<EventLoopThreadPtr> threads_;
    std::vector<EventLoop*> loops_;

};

} // namespace evk
