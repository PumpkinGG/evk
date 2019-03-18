#pragma once

#include "evk/inner_pre.h"
#include "evk/tcp_conn.h"
#include "evk/callbacks.h"
#include "evk/server_status.h"
#include "evk/inet_address.h"

#include <map>

namespace evk {
class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : public ServerStatus {
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;
    enum Option {
        kNoReusePort,
        kReusePort,
    };

public:
    TcpServer(EventLoop* loop, 
              const InetAddress& listen_addr,
              const std::string& name, 
              int num_thread = 0,
              Option option = kNoReusePort);
    ~TcpServer();

    // Start the server if it's not listenning
    void Start();

    // Stop the server
    // void Stop();

public:
    // Set connection callback
    void SetConnectionCallback(const ConnectionCallback& cb) {
        conn_cb_ = cb;
    }

    // Set message callback
    void SetMessageCallback(const MessageCallback& cb) {
        msg_cb_ = cb;
    }

    // Set message callback
    void SetWriteCompleteCallback(const WriteCompleteCallback& cb) {
        write_complete_cb_ = cb;
    }

private:
    void HandleNewConnection(int sockfd, const InetAddress& peer_addr);
    // standard usage of EventLoop::RunInLoop
    void RemoveConnection(const TcpConnectionPtr& conn);
    void RemoveConnectionInLoop(const TcpConnectionPtr& conn);
    // void StopInLoop();

private:
    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

    EventLoop* loop_;
    const std::string ip_port_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> thread_pool_;
    ConnectionCallback conn_cb_;
    MessageCallback msg_cb_;
    WriteCompleteCallback write_complete_cb_;

    // always in the loop thread
    int next_conn_id_;
    ConnectionMap connections_;

};

} // namespace evk
