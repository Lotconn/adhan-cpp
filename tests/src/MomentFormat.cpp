#include <MomentFormat.hpp>
#include <array>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>

using namespace Adhan;

namespace {
constexpr std::array<const char *, 12> kMonthNames = {
    "January", "February", "March",     "April",   "May",      "June",
    "July",    "August",   "September", "October", "November", "December"};

std::string pad2(int v) {
  std::ostringstream oss;
  oss << std::setw(2) << std::setfill('0') << v;
  return oss.str();
}
} // namespace

#ifndef ADHAN_USE_CTIME_FALLBACK

std::string formatInZone(
    const DateTime &date, const std::string &tzName,
    const std::string &formatStr) {
  using namespace std::chrono;

  if (!date.isValid()) {
    throw std::runtime_error("formatInZone: cannot format an invalid DateTime");
  }

  const time_zone *zone = locate_zone(tzName);
  zoned_time<system_clock::duration> zt{zone, date.raw()};
  auto local = zt.get_local_time();

  auto dp = floor<days>(local);
  year_month_day ymd{dp};
  hh_mm_ss hms{floor<seconds>(local - dp)};

  int year = static_cast<int>(ymd.year());
  int month = static_cast<int>(static_cast<unsigned>(ymd.month())); // 1-indexed
  int day = static_cast<int>(static_cast<unsigned>(ymd.day()));
  int hour24 = static_cast<int>(hms.hours().count());
  int minute = static_cast<int>(hms.minutes().count());
  int second = static_cast<int>(hms.seconds().count());

  int hour12 = hour24 % 12;
  if (hour12 == 0) {
    hour12 = 12;
  }
  bool isPM = hour24 >= 12;

  std::string result;
  size_t i = 0;
  while (i < formatStr.size()) {
    auto matches = [&](const char *token, size_t len) {
      return formatStr.compare(i, len, token) == 0;
    };

    if (matches("YYYY", 4)) {
      result += std::to_string(year);
      i += 4;
    } else if (matches("YY", 2)) {
      result += pad2(year % 100);
      i += 2;
    } else if (matches("MMMM", 4)) {
      result += kMonthNames.at(month - 1);
      i += 4;
    } else if (matches("DD", 2)) {
      result += pad2(day);
      i += 2;
    } else if (matches("HH", 2)) {
      result += pad2(hour24);
      i += 2;
    } else if (matches("mm", 2)) {
      result += pad2(minute);
      i += 2;
    } else if (matches("ss", 2)) {
      result += pad2(second);
      i += 2;
    } else if (matches("A", 1)) {
      result += (isPM ? "PM" : "AM");
      i += 1;
    } else if (matches("h", 1)) {
      result += std::to_string(hour12);
      i += 1;
    } else if (matches("M", 1)) {
      result += std::to_string(month);
      i += 1;
    } else if (matches("D", 1)) {
      result += std::to_string(day);
      i += 1;
    } else {
      result += formatStr[i];
      i += 1;
    }
  }
  return result;
}
#else
/* The fallback build tests should never use this */
std::string formatInZone(
    const DateTime &date, const std::string &tzName,
    const std::string &formatStr) {
  throw std::logic_error(
      "`formatInZone` disabled for fallback builds, cannot proceed.");
  return {};
}
#endif