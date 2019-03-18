#pragma once

#include "evk/tcp_server.h"

class EchoServer {
public:
    EchoServer(evk::EventLoop* loop, const evk::InetAddress& listener);
    void Start();

private:
    void OnConnection(const evk::TcpConnectionPtr& conn);
    void OnMessage(const evk::TcpConnectionPtr& conn,
                   evk::Buffer* buf);
private:
    evk::TcpServer base_server_;
};
