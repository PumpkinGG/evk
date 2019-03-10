#include <vector>

#include "evk/inner_pre.h"
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

private:


};

}
