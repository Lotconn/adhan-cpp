#include <JSDate.hpp>
#include <cassert>

#if defined(ADHAN_USE_CTIME_FALLBACK)
#include <ctime>
#else
#include <chrono>
#endif

namespace adhan {

namespace {

struct Fields {
  int year;
  int month; // 0-based, matching JS convention
  int day;
  int hours;
  int minutes;
  int seconds;
};

#if defined(ADHAN_USE_CTIME_FALLBACK)

// --- <ctime>-based implementation -----------------------------------------
// Thread-safe gmtime/localtime wrappers (signatures differ per platform).
#if defined(_WIN32)
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
      tm.tm_year + 1900,
      tm.tm_mon, // already 0-based
      tm.tm_mday,        tm.tm_hour, tm.tm_min, tm.tm_sec,
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

// --- <chrono>-based implementation (unchanged) -----------------------------
using namespace std::chrono;

Fields breakDownUtc(system_clock::time_point tp) {
  auto dp = floor<days>(tp);
  year_month_day ymd{dp};
  hh_mm_ss hms{floor<seconds>(tp - dp)};
  return Fields{
      static_cast<int>(ymd.year()),
      static_cast<int>(static_cast<unsigned>(ymd.month())) - 1,
      static_cast<int>(static_cast<unsigned>(ymd.day())),
      static_cast<int>(hms.hours().count()),
      static_cast<int>(hms.minutes().count()),
      static_cast<int>(hms.seconds().count()),
  };
}

Fields breakDownLocal(system_clock::time_point tp) {
  auto zt = zoned_time{current_zone(), tp};
  auto local = zt.get_local_time();
  auto dp = floor<days>(local);
  year_month_day ymd{dp};
  hh_mm_ss hms{floor<seconds>(local - dp)};
  return Fields{
      static_cast<int>(ymd.year()),
      static_cast<int>(static_cast<unsigned>(ymd.month())) - 1,
      static_cast<int>(static_cast<unsigned>(ymd.day())),
      static_cast<int>(hms.hours().count()),
      static_cast<int>(hms.minutes().count()),
      static_cast<int>(hms.seconds().count()),
  };
}

#endif

} // namespace

JSDate::JSDate() : JSDate(std::chrono::system_clock::now()) {}

#if defined(ADHAN_USE_CTIME_FALLBACK)

JSDate::JSDate(int year, int month, int day, int hours, int minutes,
               int seconds) {
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

JSDate::JSDate(int year, int month, int day, int hours, int minutes,
               int seconds) {
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

JSDate JSDate::now() { return JSDate(std::chrono::system_clock::now()); }

JSDate JSDate::invalid() {
  JSDate d(std::chrono::system_clock::time_point{});
  d.valid_ = false;
  return d;
}

int JSDate::getFullYear() const { return breakDownLocal(tp_).year; }
int JSDate::getMonth() const { return breakDownLocal(tp_).month; }
int JSDate::getDate() const { return breakDownLocal(tp_).day; }
int JSDate::getHours() const { return breakDownLocal(tp_).hours; }
int JSDate::getMinutes() const { return breakDownLocal(tp_).minutes; }
int JSDate::getSeconds() const { return breakDownLocal(tp_).seconds; }

int JSDate::getUTCFullYear() const { return breakDownUtc(tp_).year; }
int JSDate::getUTCMonth() const { return breakDownUtc(tp_).month; }
int JSDate::getUTCDate() const { return breakDownUtc(tp_).day; }
int JSDate::getUTCHours() const { return breakDownUtc(tp_).hours; }
int JSDate::getUTCMinutes() const { return breakDownUtc(tp_).minutes; }
int JSDate::getUTCSeconds() const { return breakDownUtc(tp_).seconds; }

long long JSDate::getTime() const {
  using namespace std::chrono;
  assert(valid_ &&
         "getTime() called on an invalid JSDate; check isValid() first");
  return duration_cast<milliseconds>(tp_.time_since_epoch()).count();
}

bool operator==(const JSDate &lhs, const JSDate &rhs) {
  if (!lhs.valid_ || !rhs.valid_)
    return false;
  return lhs.tp_ == rhs.tp_;
}

bool operator!=(const JSDate &lhs, const JSDate &rhs) {
  if (!lhs.valid_ || !rhs.valid_)
    return true;
  return lhs.tp_ != rhs.tp_;
}

bool operator<(const JSDate &lhs, const JSDate &rhs) {
  if (!lhs.valid_ || !rhs.valid_)
    return false;
  return lhs.tp_ < rhs.tp_;
}

bool operator<=(const JSDate &lhs, const JSDate &rhs) {
  if (!lhs.valid_ || !rhs.valid_)
    return false;
  return lhs.tp_ <= rhs.tp_;
}

bool operator>(const JSDate &lhs, const JSDate &rhs) {
  if (!lhs.valid_ || !rhs.valid_)
    return false;
  return lhs.tp_ > rhs.tp_;
}

bool operator>=(const JSDate &lhs, const JSDate &rhs) {
  if (!lhs.valid_ || !rhs.valid_)
    return false;
  return lhs.tp_ >= rhs.tp_;
}

} // namespace adhan