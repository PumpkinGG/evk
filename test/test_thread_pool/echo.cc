#include "echo.h"

EchoServer::EchoServer(evk::EventLoop* loop,
                       const evk::InetAddress& listener)
    : base_server_(loop, listener, "EchoServer") {
    base_server_.SetConnectionCallback(
            std::bind(&EchoServer::OnConnection, this, 
                      std::placeholders::_1));
    base_server_.SetMessageCallback(
            std::bind(&EchoServer::OnMessage, this, 
                      std::placeholders::_1,
                      std::placeholders::_2));
}

void EchoServer::Start() {
    base_server_.Start();
}

void EchoServer::OnConnection(const evk::TcpConnectionPtr& conn) {
    LOG_INFO << "EchoServer - " << conn->PeerAddress().ToIpPort().ToString() << " -> "
             << conn->LocalAddress().ToIpPort().ToString() << " is "
             << (conn->IsConnected() ? "UP" : "DOWN");
}

void EchoServer::OnMessage(const evk::TcpConnectionPtr& conn,
                           evk::Buffer* buf) {
    std::string msg(buf->NextAllString());
    LOG_INFO << conn->Name() << " echo " << msg.size() << " bytes";
    conn->Send(msg);
}
