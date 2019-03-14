#include "evk/event_loop.h"
#include "evk/timestamp.h"

#include <stdio.h>
#include <unistd.h>

evk::EventLoop* g_loop;

void thr_fn() {
    sleep(3);
    printf("\nthread 2 runs\n");
    g_loop->RunAfter(5, [&](){
               printf("\nThread 2 added timer triggers\n");
            });
    g_loop->RunAfter(8, [&](){
                printf("\nThread 2 try to stop g_loop\n");
                g_loop->Stop();
            });
}

int main() {
    evk::EventLoop loop;
    g_loop = &loop;
    std::thread thr2(thr_fn);
    loop.Run();
    thr2.join();
    return 0;
}
