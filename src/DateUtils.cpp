#include "DateUtils.hpp"
#include <chrono>

namespace adhan {

JSDate dateByAddingDays(const JSDate &date, int days) {
  if (!date.isValid()) {
    return JSDate::invalid();
  }
  /**
   * JSDate's constructor normalizes overflow the same way JS's `new Date(...)`
   * does (e.g. day 32 rolls into next month), so we can just add `days`
   * directly to getDate() without any manual carry logic.
   */
  return JSDate(date.getFullYear(), date.getMonth(), date.getDate() + days,
                date.getHours(), date.getMinutes(), date.getSeconds());
}

JSDate dateByAddingMinutes(const JSDate &date, double minutes) {
  return dateByAddingSeconds(date, minutes * 60);
}

JSDate dateByAddingSeconds(const JSDate &date, double seconds) {
  if (!date.isValid()) {
    return JSDate::invalid();
  }
  using namespace std::chrono;
  auto delta = duration_cast<system_clock::duration>(duration<double>(seconds));
  return JSDate(date.raw() + delta);
}

JSDate roundedMinute(const JSDate &date, Rounding rounding) {
  if (!date.isValid()) {
    return JSDate::invalid();
  }

  int seconds = date.getUTCSeconds();

  int offset = (seconds >= 30) ? (60 - seconds) : (-seconds);
  if (rounding == Rounding::Up) {
    offset = 60 - seconds;
  } else if (rounding == Rounding::None) {
    offset = 0;
  }

  return dateByAddingSeconds(date, offset);
}

bool isLeapYear(int year) {
  if (year % 4 != 0) {
    return false;
  }
  if (year % 100 == 0 && year % 400 != 0) {
    return false;
  }
  return true;
}

int dayOfYear(const JSDate &date) {
  int year = date.getFullYear();
  int feb = isLeapYear(year) ? 29 : 28;
  int months[] = {31, feb, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  int result = 0;
  for (int i = 0; i < date.getMonth(); i++) {
    result += months[i];
  }
  result += date.getDate();

  return result;
}

bool isValidDate(const JSDate &date) { return date.isValid(); }
} // namespace adhan