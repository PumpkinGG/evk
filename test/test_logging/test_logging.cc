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

    LOG_INFO << "This is a log of INFO!";
    LOG_DEBUG << "There is a bug!";
    LOG_ERROR << "Can not recover from Error!";
   
    return 0;
}
