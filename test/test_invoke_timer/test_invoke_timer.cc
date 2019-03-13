#include "evk/event_loop.h"
#include "evk/timestamp.h"

#include <stdio.h>
#include <unistd.h>

evk::EventLoop* g_loop;

int main() {
    evk::EventLoop loop;
    loop.RunEvery(2, [](){
                printf("RunEvery 2 seconds.\n");
            });
    loop.RunAfter(8, [&](){
                loop.Stop();
            });
    loop.Run();
    return 0;
}
