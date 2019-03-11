#pragma once

#include <time.h>
#include <stdint.h>

#include <sys/time.h>

namespace evk {
inline double utcsecond() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (double)(tv.tv_sec) + ((double)(tv.tv_usec)) / 1000000.0f;
}

inline uint64_t utcmicrosecond() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (uint64_t)(((uint64_t)(tv.tv_sec)) * 1000000 + tv.tv_usec);
}

inline struct timeval timevalconv(uint64_t time_us) {
    struct timeval tv;
    tv.tv_sec  = (long)time_us / 1000000;
    tv.tv_usec = (long)time_us % 1000000;
    return tv;
}


} // namespace evk
