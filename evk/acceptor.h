#include "evk/inner_pre.h"
#include "evk/channel.h"
#include "evk/socket.h"

namespace evk {
class EventLoop;
class InetAddress;

//
// Acceptor of incoming TCP connections
//
class Acceptor {
public:
    typedef std::function<void(int, const InetAddress&)> NewConnectionCallback;

public:
    Acceptor(EventLoop* loop, 
             const InetAddress& listen_addr, bool reuse_port = false);
    ~Acceptor();
    // set callback when connected
    void SetNewConnectionCallback(const NewConnectionCallback& cb);
    // start listening
    void Listen();
    bool IsListenning() const;

private:
    void HandleRead();

private:
    EventLoop* loop_;
    Socket accept_socket_;
    Channel accept_channel_;
    NewConnectionCallback new_connection_callback_;
    int idlefd_; // take a idle fd
    bool listenning_;

};

} // namespace evk
