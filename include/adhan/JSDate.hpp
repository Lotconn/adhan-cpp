#ifndef JSDATE_HPP
#define JSDATE_HPP

#include <chrono>

namespace adhan {

class JSDate {
public:
  /**
   * Wrap an existing time point directly
   * (UTC-based, like JS's internal epoch value)
   */
  explicit JSDate(std::chrono::system_clock::time_point tp) : tp_(tp) {}

  JSDate();

  /**
   * Mimics `new Date(year, month, day, hours, minutes, seconds)`.
   * Components are interpreted as LOCAL time, exactly like JS, and overflow
   * is normalized the same way (e.g. day 32 rolls into the next month).
   */
  JSDate(int year, int month, int day, int hours = 0, int minutes = 0,
         int seconds = 0);

  static JSDate now();

  /**
   * Returns a JSDate representing JS's "Invalid Date" state, mirroring what
   * happens when a Date is constructed from NaN (e.g. `new Date(NaN)`).
   * All comparison operators against an invalid JSDate return false, just
   * like comparisons against NaN in JS.
   */
  static JSDate invalid();

  bool isValid() const { return valid_; }

  int getFullYear() const;
  int getMonth() const;
  int getDate() const;
  int getHours() const;
  int getMinutes() const;
  int getSeconds() const;

  int getUTCFullYear() const;
  int getUTCMonth() const;
  int getUTCDate() const;
  int getUTCHours() const;
  int getUTCMinutes() const;
  int getUTCSeconds() const;

  /**
   * Milliseconds since epoch, like JS's getTime(). Behavior is undefined
   * (and asserts, in debug builds) if called on an invalid JSDate — check
   * isValid() first, the same way JS code would check isNaN(date.getTime()).
   */
  long long getTime() const;

  std::chrono::system_clock::time_point raw() const { return tp_; }

  friend bool operator==(const JSDate &lhs, const JSDate &rhs);
  friend bool operator!=(const JSDate &lhs, const JSDate &rhs);
  friend bool operator<(const JSDate &lhs, const JSDate &rhs);
  friend bool operator<=(const JSDate &lhs, const JSDate &rhs);
  friend bool operator>(const JSDate &lhs, const JSDate &rhs);
  friend bool operator>=(const JSDate &lhs, const JSDate &rhs);

  /* Boolean getter to check if we are using a fallback or not */
  bool isUsingFallback();

private:
  std::chrono::system_clock::time_point tp_{};
  bool valid_ = true;

#if defined(ADHAN_USE_CTIME_FALLBACK)
  bool fallback = true;
#else
  bool fallback = false;
#endif
};
} // namespace adhan

#endif // JSDATE_HPP