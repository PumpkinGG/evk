#pragma once

#include "evk/inner_pre.h"
#include "evk/callbacks.h"
#include "evk/inet_address.h"
#include "evk/slice.h"

#include <atomic>
#include <boost/any.hpp>

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace evk {
class EventLoop;
class Channel;
class Socket;

//
// TCP connection, for both client & server
//
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    enum Type {
        kIncoming = 0, // The type of a TCPConnction held by a TCPServer
        kOutgoing = 1, // The type of a TCPConnction held by a TCPClient
    };
    enum Status {
        kDisconnected = 0,
        kConnecting = 1,
        kConnected = 2,
        kDisconnecting = 3,
    };

public:
    TcpConnection(EventLoop* loop,
                  const std::string& name,
                  int sockfd,
                  const InetAddress& local,
                  const InetAddress& peer);
    ~TcpConnection();

    void SetConnectionCallback(const ConnectionCallback& cb) {
        conn_cb_ = cb;
    }
    void SetMessageCallback(const MessageCallback& cb) {
        msg_cb_ = cb;
    }

protected:
    friend class TCPServer;
    friend class TCPClient;
    // these methods are visible only for TCPServer and TCPClient
    // called when TcpServer accepts a new connection
    void OnConnectEstablished();   // should be called only once
    // called when TcpServer has removed me from its map
    void OnConnectDestroyed();  // should be called only once
    void SetCloseCallback(const CloseCallback& cb) {
        close_cb_ = cb;
    }
    void SetType(Type t) {
        type_ = t;
    }
    void SetStatus(Status s) {
        status_ = s;
    }

private:
    void HandleRead();
    void HandleWrite();
    void HandleClose();
    void HandleError();

private:
    EventLoop* loop_;
    std::string name_;
    Type type_;
    std::atomic<Status> status_;
    // we don't expose those classes to client
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    // address of src & dest
    InetAddress local_addr_;
    InetAddress peer_addr_;
    // callbacks
    ConnectionCallback conn_cb_;
    MessageCallback msg_cb_;
    CloseCallback close_cb_;

};

} // namespace evk
