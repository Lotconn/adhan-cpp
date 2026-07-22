#ifndef JSDATE_HPP
#define JSDATE_HPP

#include <chrono>

class JSDate {
public:
  // Wrap an existing time point directly (UTC-based, like JS's internal epoch)
  explicit JSDate(std::chrono::system_clock::time_point tp) : tp_(tp) {}

  /**
   * Mimics `new Date(year, month, day, hours, minutes, seconds)`.
   * Components are interpreted as LOCAL time, exactly like JS, and overflow
   * is normalized the same way (e.g. day 32 rolls into the next month).
   */
  JSDate(int year, int month, int day, int hours = 0, int minutes = 0,
         int seconds = 0);

  static JSDate now();

  /* Local-time getters (mirrors JS's getFullYear/getMonth/etc.) */
  int getFullYear() const;
  /* Month is 0-indexed, like JS */
  int getMonth() const;
  int getDate() const;
  int getHours() const;
  int getMinutes() const;
  int getSeconds() const;

  /* UTC getters (mirrors JS's getUTC*()) */
  int getUTCFullYear() const;
  /* Month is 0-indexed, like JS */
  int getUTCMonth() const;
  int getUTCDate() const;
  int getUTCHours() const;
  int getUTCMinutes() const;
  int getUTCSeconds() const;

  /* Milliseconds since epoch, like JS's getTime() */
  long long getTime() const;

  std::chrono::system_clock::time_point raw() const { return tp_; }

private:
  std::chrono::system_clock::time_point tp_;
};

#endif /* JSDATE_HPP */