#ifndef UTCTIME_HPP
#define UTCTIME_HPP

#include <adhan/DateUtils.hpp>

#include <chrono>

using namespace Adhan;

/**
 * Test-only helper. True when @p time falls inside the given UTC minute.
 *
 * Expected times are written as UTC here, with the local wall clock reading
 * they came from kept in a comment above each check. The conversion was done
 * once, ahead of time, by tools/generate_utc_fixtures.py. Doing it that way
 * rather than at run time means the tests need no time zone database, which
 * is what lets them build on libc++ and anywhere else the tzdb is missing.
 *
 * The comparison stops at the minute on purpose. Prayer times carry a
 * fraction of a second left over from the night length arithmetic, and that
 * fraction shifts under -Ofast, so pinning it would make the tests brittle
 * for no gain. Minute precision is also what the old formatInZone checks
 * asserted, so nothing has been tightened or loosened here.
 *
 * @param time Instant under test. An absent time never matches.
 * @param date Expected UTC calendar day.
 * @param timeOfDay Expected time of day, UTC, e.g. `8h + 42min`.
 */
inline bool isUtc(
    const OptInstant &time, const std::chrono::year_month_day &date,
    const std::chrono::minutes &timeOfDay) {
  if (!time) {
    return false;
  }
  return std::chrono::floor<std::chrono::minutes>(*time) ==
         std::chrono::sys_days{date} + timeOfDay;
}

#endif // UTCTIME_HPP
