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

    LOG_INFO << "This is INFO!";
    LOG_DEBUG << "For Debug!";
    LOG_ERROR << "For Error!";
   
    return 0;
}
