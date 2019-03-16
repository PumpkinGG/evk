#pragma once

#include "evk/inner_pre.h"
#include "evk/callbacks.h"
#include "evk/server_status.h"
#include "evk/inet_address.h"

#include <map>

namespace evk {
class Acceptor;
class EventLoop;

class TCPServer : public ServerStatus {
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;
    enum Option {
        kNoReusePort,
        kReusePort,
    };

public:
    TCPServer(EventLoop* loop, const InetAddress& listen_addr,
              const std::string& name, Option option = kNoReusePort);
    ~TCPServer();

    // Start the server if it's not listenning
    void Start();

    // Stop the server
    void Stop();

public:
    // Set connection callback
    void SetConnectionCallback(const ConnectionCallback& cb) {
        conn_cb_ = cb;
    }

    // Set message callback
    void SetMessageCallback(const MessageCallback& cb) {
        msg_cb_ = cb;
    }

private:
    void HandleNewConnection(int sockfd, const InetAddress& peer_addr);

private:
    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

    EventLoop* loop_;
    const std::string ip_port_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    ConnectionCallback conn_cb_;
    MessageCallback msg_cb_;

    // always in the loop thread
    int next_conn_id_;
    ConnectionMap connections_;

};

} // namespace evk
