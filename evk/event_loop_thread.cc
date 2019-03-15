#include "evk/inner_pre.h"

#include "evk/event_loop.h"
#include "evk/event_loop_thread.h"

namespace evk {
EventLoopThread::EventLoopThread(const InitCallback& init_cb,
                                 const std::string name)
    : event_loop_(nullptr), name_(name), init_cb_(init_cb) {
    DLOG_TRACE << "loop = " << event_loop_;
}

EventLoopThread::~EventLoopThread() {
    assert(IsRunning());
    Join();
}

EventLoop* EventLoopThread::StartLoop() {
    DLOG_TRACE;
    status_.store(kStarting);

    assert(thread_.get() == nullptr);
    thread_.reset(new std::thread(std::bind(&EventLoopThread::Run, this)));
    
    // waiting for completing initialization and running
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (status_.load() < kRunning) {
            cond_.wait(lock);
        }
    }
    return event_loop_.get();
}

void EventLoopThread::Run() {
    event_loop_.reset(new EventLoop);
    DLOG_TRACE << "loop = " << event_loop_;
    if (name_.empty()) {
        std::ostringstream os;
        os << "thread-" << std::this_thread::get_id();
        name_ = os.str();
    }

    DLOG_TRACE << "loop = " << event_loop_ << " execute pre_cb";
    if (init_cb_) {
        init_cb_(event_loop_.get());
    }
    // notify StartLoop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        status_.store(kRunning);
        cond_.notify_one();
    }
    event_loop_->Run();

    // Stopping
    assert(event_loop_->IsStopped());
    DLOG_TRACE << "loop = " << event_loop_ << " EventLoopThread stopped";
    status_.store(kStopped);
    event_loop_.reset();
}

void EventLoopThread::Stop() {
    DLOG_TRACE << "loop = " << event_loop_;
    assert(status_ == kRunning && IsRunning());
    status_.store(kStopping);
    event_loop_->Stop();

    // waiting for stopping
    while (!IsStopped()) {
        usleep(1);
    }
    DLOG_TRACE << "loop = " << event_loop_ << " thread stopped";
    Join();
    DLOG_TRACE << "loop = " << event_loop_ << " thread thread totally stopped";
}

void EventLoopThread::Join() {
    // To avoid multi other threads call Join simultaneously
    std::unique_lock<std::mutex> lock(mutex_);
    if (thread_ && thread_->joinable()) {
        DLOG_TRACE << "loop = " << event_loop_ << " thread = " << thread_ << " joinable";
        try {
            thread_->join();
        } catch (const std::system_error& e) {
            LOG_ERROR << "Caught a system_error: " << e.what() << " code = " << e.code();
        }
        thread_.reset();
    }
}

bool EventLoopThread::IsRunning() const {
    // Using event_loop_->IsRunning() is more exact to query where thread is
    // running or not instead of status_ == kRunning
    //
    // Because in some particular circumstances,
    // when status_==kRunning and event_loop_::running_ == false,
    // the application will broke down
    return event_loop_->IsRunning();
}

const std::string& EventLoopThread::name() const {
    return name_;
}

std::thread::id EventLoopThread::tid() const {
    if (thread_) {
        return thread_->get_id();
    }
    return std::thread::id();
}

} // namespace evk
