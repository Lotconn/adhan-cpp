#include <adhan/DateTime.hpp>
#include <cassert>
#include <stdexcept>

#ifdef ADHAN_USE_CTIME_FALLBACK
#include <ctime>
#else
#include <chrono>
#endif

namespace Adhan {

namespace {

struct Fields {
  int year;
  int month; // 0-based, matching JS convention
  int day;
  int hours;
  int minutes;
  int seconds;
};

#ifdef ADHAN_USE_CTIME_FALLBACK

// Thread-safe gmtime/localtime wrappers (signatures differ per platform).
#ifdef _WIN32
std::tm portableGmtime(std::time_t t) {
  std::tm out{};
  gmtime_s(&out, &t);
  return out;
}
std::tm portableLocaltime(std::time_t t) {
  std::tm out{};
  localtime_s(&out, &t);
  return out;
}
#else
std::tm portableGmtime(std::time_t t) {
  std::tm out{};
  gmtime_r(&t, &out);
  return out;
}
std::tm portableLocaltime(std::time_t t) {
  std::tm out{};
  localtime_r(&t, &out);
  return out;
}
#endif

Fields fieldsFromTm(const std::tm &tm) {
  return Fields{
      .year = tm.tm_year + 1900,
      .month = tm.tm_mon, // already 0-based
      .day = tm.tm_mday,
      .hours = tm.tm_hour,
      .minutes = tm.tm_min,
      .seconds = tm.tm_sec,
  };
}

Fields breakDownUtc(std::chrono::system_clock::time_point tp) {
  std::time_t t = std::chrono::system_clock::to_time_t(tp);
  return fieldsFromTm(portableGmtime(t));
}

Fields breakDownLocal(std::chrono::system_clock::time_point tp) {
  std::time_t t = std::chrono::system_clock::to_time_t(tp);
  return fieldsFromTm(portableLocaltime(t));
}

#else

using namespace std::chrono;

Fields breakDownUtc(system_clock::time_point tp) {
  auto dp = floor<days>(tp);
  year_month_day ymd{dp};
  hh_mm_ss hms{floor<seconds>(tp - dp)};
  return Fields{
      .year = static_cast<int>(ymd.year()),
      .month = static_cast<int>(static_cast<unsigned>(ymd.month())) - 1,
      .day = static_cast<int>(static_cast<unsigned>(ymd.day())),
      .hours = static_cast<int>(hms.hours().count()),
      .minutes = static_cast<int>(hms.minutes().count()),
      .seconds = static_cast<int>(hms.seconds().count()),
  };
}

Fields breakDownLocal(system_clock::time_point tp) {
  auto zt = zoned_time{current_zone(), tp};
  auto local = zt.get_local_time();
  auto dp = floor<days>(local);
  year_month_day ymd{dp};
  hh_mm_ss hms{floor<seconds>(local - dp)};
  return Fields{
      .year = static_cast<int>(ymd.year()),
      .month = static_cast<int>(static_cast<unsigned>(ymd.month())) - 1,
      .day = static_cast<int>(static_cast<unsigned>(ymd.day())),
      .hours = static_cast<int>(hms.hours().count()),
      .minutes = static_cast<int>(hms.minutes().count()),
      .seconds = static_cast<int>(hms.seconds().count()),
  };
}

#endif

} // namespace

DateTime::DateTime() : DateTime(std::chrono::system_clock::now()) {}

#ifdef ADHAN_USE_CTIME_FALLBACK

DateTime::DateTime(
    int year, int month, int day, int hours, int minutes, int seconds) {
  if (year < -32767 || year > 32767) {
    throw std::invalid_argument(
        "DateTime: year is outside the representable range");
  }
  std::tm tm{};
  tm.tm_year = year - 1900;
  tm.tm_mon = month; // 0-based, matches mktime's expectation
  tm.tm_mday = day;
  tm.tm_hour = hours;
  tm.tm_min = minutes;
  tm.tm_sec = seconds;
  // NOTE: tm_isdst = -1 lets mktime resolve DST itself. Unlike
  // choose::earliest, the exact resolution for ambiguous (fall-back) or
  // nonexistent (spring-forward) local times is implementation-defined,
  // not guaranteed to be "earliest". This only affects the ~1-2 hour
  // DST-transition windows twice a year.
  tm.tm_isdst = -1;

  std::time_t t = std::mktime(&tm);
  tp_ = std::chrono::system_clock::from_time_t(t);
}

#else

DateTime::DateTime(
    int year, int month, int day, int hours, int minutes, int seconds) {
  if (year < -32767 || year > 32767) {
    throw std::invalid_argument(
        "DateTime: year is outside the representable range");
  }
  using namespace std::chrono;

  auto base = local_days{std::chrono::year{year} /
                         std::chrono::month{static_cast<unsigned>(month + 1)} /
                         std::chrono::day{1}};
  auto localTime = base + std::chrono::days{day - 1} +
                   std::chrono::hours{hours} + std::chrono::minutes{minutes} +
                   std::chrono::seconds{seconds};

  auto zt = zoned_time{current_zone(), localTime, choose::earliest};
  tp_ = zt.get_sys_time();
}

#endif

DateTime DateTime::now() {
  return DateTime(std::chrono::system_clock::now());
}

DateTime DateTime::invalid() {
  DateTime d(std::chrono::system_clock::time_point{});
  d.valid_ = false;
  return d;
}

int DateTime::getFullYear() const {
  return breakDownLocal(tp_).year;
}
int DateTime::getMonth() const {
  return breakDownLocal(tp_).month;
}
int DateTime::getDate() const {
  return breakDownLocal(tp_).day;
}
int DateTime::getHours() const {
  return breakDownLocal(tp_).hours;
}
int DateTime::getMinutes() const {
  return breakDownLocal(tp_).minutes;
}
int DateTime::getSeconds() const {
  return breakDownLocal(tp_).seconds;
}

int DateTime::getUTCFullYear() const {
  return breakDownUtc(tp_).year;
}
int DateTime::getUTCMonth() const {
  return breakDownUtc(tp_).month;
}
int DateTime::getUTCDate() const {
  return breakDownUtc(tp_).day;
}
int DateTime::getUTCHours() const {
  return breakDownUtc(tp_).hours;
}
int DateTime::getUTCMinutes() const {
  return breakDownUtc(tp_).minutes;
}
int DateTime::getUTCSeconds() const {
  return breakDownUtc(tp_).seconds;
}

long long DateTime::getTime() const {
  using namespace std::chrono;
  assert(
      valid_ &&
      "getTime() called on an invalid DateTime; check isValid() first");
  return duration_cast<milliseconds>(tp_.time_since_epoch()).count();
}

bool DateTime::isUsingFallback() const {
  return this->fallback;
}

bool operator==(const DateTime &lhs, const DateTime &rhs) {
  if (!lhs.valid_ || !rhs.valid_) {
    return false;
  }
  return lhs.tp_ == rhs.tp_;
}

bool operator!=(const DateTime &lhs, const DateTime &rhs) {
  if (!lhs.valid_ || !rhs.valid_) {
    return true;
  }
  return lhs.tp_ != rhs.tp_;
}

bool operator<(const DateTime &lhs, const DateTime &rhs) {
  if (!lhs.valid_ || !rhs.valid_) {
    return false;
  }
  return lhs.tp_ < rhs.tp_;
}

bool operator<=(const DateTime &lhs, const DateTime &rhs) {
  if (!lhs.valid_ || !rhs.valid_) {
    return false;
  }
  return lhs.tp_ <= rhs.tp_;
}

bool operator>(const DateTime &lhs, const DateTime &rhs) {
  if (!lhs.valid_ || !rhs.valid_) {
    return false;
  }
  return lhs.tp_ > rhs.tp_;
}

bool operator>=(const DateTime &lhs, const DateTime &rhs) {
  if (!lhs.valid_ || !rhs.valid_) {
    return false;
  }
  return lhs.tp_ >= rhs.tp_;
}

} // namespace Adhan