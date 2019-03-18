# ***evk***
* A simple C++ network library, Referring evpp & muduo
* Based on Boost, Glog

## *schdule*
* Completed: 
   * Reactor Event Loop based on poll(2). 
      * EventLoop class for event driver loop, 
      * Channel class ties a fixed fd, managing events and callbacks on such fd.
   * EventLoop RunInLoop method to ensure running callbacks in loop.
   * Timer Queue based on timerfd & RunInLoop.
   * Wrapper socket API & sockaddr, sockaddr_in, sockaddr_in6 into Socket class & InetAddress class.
   * TcpConnection class maintains a fixed connection.
   * TcpServer class based on a single thread.
   * Expanding TcpServer class with multi threads. 
      * Based on EventLoopThread and EventLoopThreadPool.

