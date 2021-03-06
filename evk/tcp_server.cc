#include "evk/inner_pre.h"
#include "evk/tcp_server.h"
#include "evk/tcp_conn.h"
#include "evk/event_loop.h"
#include "evk/acceptor.h"
#include "evk/event_loop_thread_pool.h"
#include "evk/socket_ops.h"

#include <stdio.h>

namespace evk {
TcpServer::TcpServer(EventLoop* loop, 
                     const InetAddress& listen_addr,
                     const std::string& name, 
                     int num_thread,
                     Option option) 
    : loop_(loop),
      ip_port_(listen_addr.ToIpPort()),
      name_(name),
      acceptor_(new Acceptor(loop, listen_addr, option == kReusePort)),
      thread_pool_(new EventLoopThreadPool(loop, name_)),
      conn_cb_(internal::DefaultConnectionCallback),
      msg_cb_(internal::DefaultMessageCallback),
      next_conn_id_(1) {
    DLOG_TRACE;
    status_.store(kInitializing);
    acceptor_->SetNewConnectionCallback(
            std::bind(&TcpServer::HandleNewConnection, this, 
            std::placeholders::_1, std::placeholders::_2));
    if (num_thread < 0) num_thread = 0;
    thread_pool_->SetThreadNum(num_thread);
    status_.store(kInitialized);
}

TcpServer::~TcpServer() {
    loop_->AssertInLoopThread();
    DLOG_TRACE;

    for (auto& item: connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->GetLoop()->RunInLoop(
                std::bind(&TcpConnection::OnConnectDestroyed, conn));
    }
}

void TcpServer::Start() {
    DLOG_TRACE;
    Status expected(kInitialized);
    if (status_.compare_exchange_strong(expected, kStarting)) {
        thread_pool_->Start();
        assert(!acceptor_->IsListenning());
        loop_->RunInLoop(
                std::bind(&Acceptor::Listen, acceptor_.get()));
        status_.store(kRunning);
    }
}

void TcpServer::HandleNewConnection(int sockfd, const InetAddress& peer_addr) {
    loop_->AssertInLoopThread();
    EventLoop* ioLoop = thread_pool_->GetNextLoop();
    char buf[64];
    snprintf(buf, sizeof(buf), "-%s#%d", ip_port_.c_str(), next_conn_id_);
    ++next_conn_id_;
    std::string connName = name_ + buf;

    LOG_INFO << "TcpServer::HandleNewConnection [" << name_
             << "] - new connection [" << connName
             << "] from " << peer_addr.ToIpPort();

    InetAddress localAddr(sock::GetLocalAddr(sockfd));
    TcpConnectionPtr conn(
            new TcpConnection(ioLoop, connName, sockfd, localAddr, peer_addr));
    connections_[connName] = conn;
    conn->SetConnectionCallback(conn_cb_);
    conn->SetMessageCallback(msg_cb_);
    conn->SetWriteCompleteCallback(write_complete_cb_);
    // when disconnecting, 
    // need to callback TcpServer member to erase shared_ptr in map,
    // then TcpConnection will do something(remove channel and so on) and destruct.
    conn->SetCloseCallback(
            std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));
    ioLoop->RunInLoop(
            std::bind(&TcpConnection::OnConnectEstablished, conn));
}

void TcpServer::RemoveConnection(const TcpConnectionPtr& conn) {
    loop_->RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr& conn) {
    loop_->AssertInLoopThread();
    LOG_INFO << "TcpServer::RemoveConnectionInLoop [" << name_
             << "] - connection " << conn->Name();
    size_t n = connections_.erase(conn->Name());
    assert(n == 1); (void)n;
    EventLoop* ioLoop = conn->GetLoop();
    ioLoop->QueueInLoop(std::bind(&TcpConnection::OnConnectDestroyed, conn));
}

} // namespace evk
