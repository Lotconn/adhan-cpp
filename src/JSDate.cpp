#include <JSDate.hpp>
#include <cassert>

namespace adhan {

namespace {
using namespace std::chrono;

struct Fields {
  int year;
  int month;
  int day;
  int hours;
  int minutes;
  int seconds;
};

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
} // namespace

JSDate::JSDate() : JSDate(std::chrono::system_clock::now()) {}

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
    return false; // NaN != NaN, mirroring JS
  return lhs.tp_ == rhs.tp_;
}

bool operator!=(const JSDate &lhs, const JSDate &rhs) {
  if (!lhs.valid_ || !rhs.valid_)
    return true; // NaN != NaN, mirroring JS
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