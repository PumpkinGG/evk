#pragma once

#include "evk/inner_pre.h"
#include "evk/timestamp.h"
#include "evk/buffer.h"

namespace evk {
class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void()> TimerCallback;

typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void(const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;

typedef std::function<void(const TcpConnectionPtr&, Buffer*)> MessageCallback;

namespace internal {
inline void DefaultConnectionCallback(const TcpConnectionPtr&) {
    LOG_TRACE << "DefaultConnectionCallback";
}
inline void DefaultMessageCallback(const TcpConnectionPtr&, Buffer* buffer) {
    buffer->Reset();
}

} // namespace internal
} // namespace evk
