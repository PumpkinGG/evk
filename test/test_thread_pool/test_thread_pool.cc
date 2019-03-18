#include "echo.h"
#include "evk/event_loop.h"
#include "evk/logging.h"

#include <unistd.h>

int main(int argc, char* argv[]) {
    FLAGS_logtostderr = true;
    google::InitGoogleLogging(argv[0]);

    LOG_INFO << "pid = " << getpid();
    evk::EventLoop loop;
    evk::InetAddress listener(8848);
    EchoServer server(&loop, listener, 3);

    server.Start();
    loop.Run();
}

