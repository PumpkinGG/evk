#include <vector>
#include <map>

#include "evk/inner_pre.h"
#include "evk/timestamp.h"
#include "evk/event_loop.h"

struct pollfd;

namespace evk {
class Channel;

// IO Multiplexing with poll(2).
// This class doesn't own the Channel objects.
class Poller: public noncopyable {
public:
    typedef std::vector<Channel*> ChannelList;

public:
    Poller(EventLoop* loop);
    ~Poller();
    
    // Polls the IO events.
    // Must be called in the loop thread.
    Timestamp poll(int timeout_ms, ChannelList* active_channels);

    // Changes the interested IO events.
    // Must be called in the loop thread.
    void UpdateChannel(Channel* channel);

    void AssertInLoopThread() {
        owner_loop_->AssertInLoopThread();
    }

private:
    void FillActiveChannels(int num_events, ChannelList* active_channels);

private:
    typedef std::vector<struct pollfd> PollfdList;
    typedef std::map<int, Channel*> ChannelMap;

    EventLoop* owner_loop_;
    PollfdList pollfd_list_;
    ChannelMap channel_map_;

};

}
