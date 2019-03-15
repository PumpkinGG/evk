#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>

#include "evk/inner_pre.h"
#include "evk/server_status.h"

namespace evk {
class EventLoop;

class EventLoopThread : public ServerStatus {
public:
    // invoked when initializing
    typedef std::function<void(EventLoop*)> InitCallback;

public:
    EventLoopThread(const InitCallback& init_cb = InitCallback(),
                    const std::string name = std::string());
    ~EventLoopThread();
    // start this event loop thread
    // called by main thread or any other thread
    EventLoop* StartLoop();

    // rarely called by usr
    void Stop();
    
    // @brief Join the thread. If you forget to call this method,
    // it will be invoked automatically in the destruct method ~EventLoopThread().
    // @note DO NOT call this method from any of the working thread.
    void Join();

public:
    bool IsRunning() const;
    const std::string& name() const;
    std::thread::id tid() const;

private:
    // main function running in the loop thread
    void Run();

private:
    std::shared_ptr<EventLoop> event_loop_; // using normal pointer
    std::string name_;
    InitCallback init_cb_;

    std::mutex mutex_;
    std::shared_ptr<std::thread> thread_; // @GardedBy mutex_
    std::condition_variable cond_; // @GardedBy mutex_
    
};

} // namespace evk
