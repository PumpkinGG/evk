#include <stdio.h>

#include "evk/event_loop_thread.h"
#include "evk/event_loop.h"

void thr_fn(evk::EventLoop* loop) {
    printf("\nevent_loop_thread init callback---run after\n");
    loop->RunInLoop([](){
                printf("\nRunInLoop invoked\n\n");
            });
}

int main() {
    evk::EventLoopThread loop_thr(thr_fn, "run_after");
    auto loop = loop_thr.StartLoop();
    loop->RunAfter(5, [&loop](){
                loop->Stop();
            });
    printf("\nmain thread set loop to stop after 5 seconds\n\n");
    return 0;
}
