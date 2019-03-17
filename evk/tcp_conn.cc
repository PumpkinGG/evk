#include "evk/inner_pre.h"
#include "evk/tcp_conn.h"
#include "evk/event_loop.h"
#include "evk/channel.h"
#include "evk/socket.h"
#include "evk/socket_ops.h"

#include <errno.h>

namespace evk {
TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& name,
                             int sockfd,
                             const InetAddress& local,
                             const InetAddress& peer)
    : loop_(loop),
      name_(name),
      type_(kIncoming),
      status_(kConnecting),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      local_addr_(local),
      peer_addr_(peer),
      high_water_mark_(64*1024*1024) {
    DLOG_TRACE << "TcpConnection::ctor[" <<  name_ << "] at " << this
               << " fd=" << sockfd;;
    channel_->SetReadCallback(
            std::bind(&TcpConnection::HandleRead, this));
    channel_->SetWriteCallback(
            std::bind(&TcpConnection::HandleWrite, this));
    channel_->SetCloseCallback(
            std::bind(&TcpConnection::HandleClose, this));
    channel_->SetErrorCallback(
            std::bind(&TcpConnection::HandleError, this));
    socket_->SetKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    DLOG_TRACE;
    assert(status_ == kDisconnected);
}

void TcpConnection::OnConnectEstablished() {
    loop_->AssertInLoopThread();
    assert(status_ == kConnecting);
    SetStatus(kConnected);
    channel_->EnableReadEvent();
    conn_cb_(shared_from_this());
}

void TcpConnection::OnConnectDestroyed() {
    loop_->AssertInLoopThread();
    if (status_ == kConnected) {
        SetStatus(kDisconnected);
        channel_->DisableAllEvent();
        conn_cb_(shared_from_this());
    }
    channel_->Remove();
}

void TcpConnection::HandleRead() {
    loop_->AssertInLoopThread();
    int savedError = 0;
    ssize_t n = input_buffer_.ReadFromFD(channel_->fd(), &savedError);
    if (n > 0) {
        msg_cb_(shared_from_this(), &input_buffer_);
    } else if (n == 0) {
        HandleClose();
    } else {
        errno = savedError;
        LOG_ERROR << "TcpConnection::HandleRead";
        HandleError();
    }
}

void TcpConnection::HandleWrite() {
    loop_->AssertInLoopThread();
    if (channel_->IsWriting()) {
        ssize_t n = sock::Write(channel_->fd(), 
                                output_buffer_.data(),
                                output_buffer_.size());
        if (n > 0) {
            output_buffer_.Skip(n);
            if (output_buffer_.size() == 0) {
                channel_->DisableWriteEvent();
                if (write_complete_cb_) {
                    loop_->QueueInLoop(
                            std::bind(write_complete_cb_, shared_from_this()));
                }
                if (status_ == kDisconnecting) {
                    ShutdownInLoop();
                }
            }
        } else {
            LOG_ERROR << "TcpConnection::HandleWrite";
        }
    } else {
        DLOG_TRACE << channel_->fd() << " is down, no more writing";
    }
}

void TcpConnection::HandleClose() {
    loop_->AssertInLoopThread();
    DLOG_TRACE << "fd = " << channel_->fd();
    assert(status_ == kConnected || status_ == kDisconnecting);
    // we don't close fd here
    SetStatus(kDisconnected);
    channel_->DisableAllEvent();
    close_cb_(shared_from_this());
}

void TcpConnection::HandleError() {
    int err = sock::GetSocketError(channel_->fd());
    LOG_ERROR << "TcpConnection::HandleError [" << name_
              << "] - SO_ERROR = " << err;
}

void TcpConnection::Send(const void* message, int len) {
    Send(Slice(static_cast<const char*>(message), len));
}

void TcpConnection::Send(const Slice& message) {
    if (status_ == kConnected) {
        if (loop_->IsInLoopThread()) {
            SendInLoop(message);
        } else {
            auto cat = shared_from_this();
            loop_->RunInLoop(
                    [cat, &message](){
                        cat->SendInLoop(message);
                    });
        }
    }
}

void TcpConnection::Send(Buffer* message) {
    if (status_ == kConnected) {
        if (loop_->IsInLoopThread()) {
            SendInLoop(message->data(), message->length());
            message->Reset();
        } else {
            auto cat = shared_from_this();
            loop_->RunInLoop(
                    [cat, &message](){
                        cat->SendInLoop(message->NextAll());
                    });
        }
    }
}

void TcpConnection::SendInLoop(const Slice& message) {
    SendInLoop(static_cast<const void*>(message.data()), message.size());
}

void TcpConnection::SendInLoop(const void* message, size_t len) {
    loop_->AssertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (status_ == kDisconnected) {
        LOG_WARN << "kDisconnected, give up writing";
        return;
    }
    // if nothing in output queue, try writing directly
    if (!channel_->IsWriting() && output_buffer_.size() == 0) {
        nwrote = sock::Write(channel_->fd(), message, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && write_complete_cb_) {
                loop_->QueueInLoop(std::bind(write_complete_cb_, shared_from_this()));
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_ERROR << "TcpConnection::SendInLoop";
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    assert(remaining <= len);
    if (!faultError && remaining > 0) {
        size_t oldLen = output_buffer_.size();
        if (oldLen + remaining >= high_water_mark_
            && oldLen < high_water_mark_
            && high_water_mark_cb_) {
            loop_->QueueInLoop(std::bind(high_water_mark_cb_, 
                               shared_from_this(), oldLen+remaining));
        }
        // !!!message shift nwrote!!!
        output_buffer_.Append(static_cast<const char*>(message)+nwrote, remaining);
        if (!channel_->IsWriting()) {
            channel_->EnableWriteEvent();
        }
    }
}

void TcpConnection::Shutdown() {
    Status expected(kConnected);
    if (status_.compare_exchange_strong(expected, kDisconnecting)) {
        auto cat = shared_from_this();
        loop_->RunInLoop([cat](){
                    cat->ShutdownInLoop();
                });
    }
}

void TcpConnection::ShutdownInLoop() {
    loop_->AssertInLoopThread();
    if (!channel_->IsWriting()) {
        socket_->ShutDownWrite();
    }
}

} // namespace evk
