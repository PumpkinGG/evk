#include "evk/event_loop.h"
#include "evk/logging.h"

#include <stdio.h>
#include <unistd.h>

void thr_fn() {
    evk::EventLoop loop;
    loop.Run();
}

int main() {
    std::thread thr1(thr_fn);
    
    evk::EventLoop loop;
    loop.Run();
    
    thr1.join();
    return 0;
}
