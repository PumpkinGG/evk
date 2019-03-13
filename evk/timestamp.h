#pragma once

#include "duration.h"
#include "gettimeofday.h"
#include <chrono>

namespace evk {
class Timestamp {
public:
    Timestamp();
    explicit Timestamp(int64_t nanoseconds);
    explicit Timestamp(const struct timeval& t);

    static Timestamp Now();
    static Timestamp Invalid();

    struct timeval TimeVal() const;
    void To(struct timeval* t) const;

    // Unix returns t as a Unix time, the number of seconds
    // elapsed since Jan 1, 1970 UTC.
    int64_t Unix() const;

    // The result may be undefined if the Unix time in nanoseconds
    // cannot be represented by an int64.
    int64_t UnixNano() const;

    // The result may be undefined if the Unix time in microseconds
    // cannot be represented by an int64.
    int64_t UnixMicro() const;
    
    // Transfom to string
    std::string ToString() const;
    std::string ToFormattedString(bool show_microseconds) const;
    
    void Add(Duration d);

    bool IsValid() const;
    bool operator< (const Timestamp& rhs) const;
    bool operator==(const Timestamp& rhs) const;

    Timestamp operator+=(const Duration& rhs);
    Timestamp operator+ (const Duration& rhs) const;
    Timestamp operator-=(const Duration& rhs);
    Timestamp operator- (const Duration& rhs) const;
    Duration  operator- (const Timestamp& rhs) const;

private:
    // ns_ gives the number of nanoseconds elapsed since the Epoch
    // 1970-01-01 00:00:00 +0000 (UTC).
    int64_t ns_;

};

inline Timestamp::Timestamp()
    : ns_(0) {}

inline Timestamp::Timestamp(int64_t nanoseconds)
    : ns_(nanoseconds) {}

inline Timestamp::Timestamp(const struct timeval& t)
    : ns_((int64_t)(t.tv_sec) * Duration::kSecond + t.tv_usec * Duration::kMicrosecond) {}

inline bool Timestamp::IsValid() const {
    return ns_ > 0;
}

inline Timestamp Timestamp::Now() {
    // gettimeofday have a higher performance
#if 1
    return Timestamp(int64_t(utcmicrosecond() * Duration::kMicrosecond));
#else
    return Timestamp(std::chrono::duration_cast<std::chrono::nanoseconds>
            (std::chrono::system_clock::now().time_since_epoch()).count());
#endif
}

inline Timestamp Timestamp::Invalid() {
    return Timestamp();
}

inline void Timestamp::Add(Duration d) {
    ns_ += d.Nanoseconds();
}

inline void Timestamp::To(struct timeval* t) const {
    t->tv_sec  = (long)(ns_ / Duration::kSecond);
    t->tv_usec = (long)(ns_ % Duration::kSecond) / (long)(Duration::kMicrosecond);
}

inline struct timeval Timestamp::TimeVal() const {
    struct timeval t;
    To(&t);
    return t;
}

inline int64_t Timestamp::Unix() const {
    return ns_ / Duration::kSecond;
}

inline int64_t Timestamp::UnixNano() const {
    return ns_;
}

inline int64_t Timestamp::UnixMicro() const {
    return ns_ / Duration::kMicrosecond;
}

inline bool Timestamp::operator< (const Timestamp& rhs) const {
    return ns_ <  rhs.ns_;
}

inline bool Timestamp::operator==(const Timestamp& rhs) const {
    return ns_ == rhs.ns_;
}

inline Timestamp Timestamp::operator+=(const Duration& rhs) {
    ns_ += rhs.Nanoseconds();
    return *this;
}

inline Timestamp Timestamp::operator+ (const Duration& rhs) const {
    Timestamp temp(*this);
    temp += rhs;
    return temp;
}

inline Timestamp Timestamp::operator-=(const Duration& rhs) {
    ns_ -= rhs.Nanoseconds();
    return *this;
}

inline Timestamp Timestamp::operator- (const Duration& rhs) const {
    Timestamp temp(*this);
    temp -= rhs;
    return temp;
}

inline Duration Timestamp::operator- (const Timestamp& rhs) const {
    int64_t ns = ns_ - rhs.ns_;
    return Duration(ns);
}

} // namespace evk
