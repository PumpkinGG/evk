#include "evk/logging.h"
#include <atomic>

int main(int argc, char* argv[]) {
    FLAGS_logtostderr = true;
    google::InitGoogleLogging(argv[0]);

    enum Weeks {
        Monday    = 1,
        Tuesday   = 2,
        Wednesday = 3,
        Thursday  = 4,
        Friday    = 5,
        Saturday  = 6,
        Sunday    = 7
    };

    std::atomic<Weeks> week;
    week.store(Monday);

    LOG_INFO << "INFO!";
    LOG_DEBUG << "Debug!";
    LOG_ERROR << "Error!";
    LOG_FATAL << "Fatal!";
   
    return 0;
}
