#include "evk/event_loop.h"
#include "evk/channel.h"
#include "evk/logging.h"

#include <sys/timerfd.h>
#include <stdio.h>

evk::EventLoop* g_loop;

void timeout() {
    printf("Timeout!\n");
    g_loop->Stop();
}

int main(int argc, char* argv[]) {
    FLAGS_logtostderr = true;
    google::InitGoogleLogging(argv[0]);
    
    evk::EventLoop loop;
    g_loop = &loop;

    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    evk::Channel channel(&loop, timerfd);
    channel.SetReadCallback(timeout);
    channel.EnableReadEvent();

    struct itimerspec how_long;
    bzero(&how_long, sizeof(how_long));
    how_long.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &how_long, NULL);

    loop.Run();
    ::close(timerfd);

    return 0;
}
