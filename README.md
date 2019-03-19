# ***evk***
* A simple C++ network library, Referring evpp & muduo
* Based on Boost, Glog

## *schedule*
* Completed: 
   * Reactor Event Loop based on ~~poll(2)~~ epoll(2)&&poll(2) -> Default epoll(2). 
      * EventLoop class for event driver loop, 
      * Channel class ties a fixed fd, managing events and callbacks on such fd.
   * EventLoop RunInLoop method to ensure running callbacks in loop.
   * Timer Queue based on timerfd & RunInLoop.
   * Wrapper socket API & sockaddr, sockaddr_in, sockaddr_in6 into Socket class & InetAddress class.
   * Acceptor class maintains socketfd's bind, listen and accept.
   * TcpConnection class maintains a fixed connection.
   * TcpServer class based on a single thread.
   * Expanding TcpServer class with multi threads. 
      * Based on EventLoopThread and EventLoopThreadPool.
      * TcpServer base_event_loop maintains Acceptor obj, managing accept events and assigning TcpConns to a EventLoopThread in thread pool.

