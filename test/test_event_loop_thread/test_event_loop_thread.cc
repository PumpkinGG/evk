#include <stdio.h>

#include "evk/event_loop_thread.h"
#include "evk/event_loop.h"

void thr_fn(evk::EventLoop* loop) {
    printf("\nevent_loop_thread init callback---run after\n\n");
    loop->RunAfter(5, [](){
                printf("\nRunAfter invoked\n\n");
            });
}

int main() {
    evk::EventLoopThread loop_thr(thr_fn, "run_after");
    auto loop = loop_thr.StartLoop();
    loop->RunAfter(8, [&loop](){
                loop->Stop();
            });
    return 0;
}
