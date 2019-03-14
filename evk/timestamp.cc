#include "evk/timestamp.h"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

namespace evk {
std::string Timestamp::ToString() const {
    char buf[32] = {0};
    int64_t seconds = ns_ / Duration::kSecond;
    int64_t microseconds = (ns_ % Duration::kSecond) / Duration::kMicrosecond;
    // PRId64 is used for compatibility of 32-bit and 64-bit os
    snprintf(buf, sizeof(buf)-1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
    return buf;
}

std::string Timestamp::ToFormattedString(bool show_microseconds) const {
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(ns_ / Duration::kSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    if (show_microseconds) {
        int microseconds = static_cast<int>((ns_ % Duration::kSecond) / Duration::kMicrosecond);
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
                                   tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                                   tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
    } else {
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
                                   tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                                   tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}

} // namespace evk
