#ifndef MOMENTFORMAT_HPP
#define MOMENTFORMAT_HPP

#include <adhan/DateUtils.hpp>
#include <string>

using namespace Adhan;

/**
 * Test-only helper. Renders an instant as it would look in the given IANA
 * time zone, using the handful of moment.js format tokens the ported test
 * suite actually asks for:
 *
 *   YYYY  4 digit year
 *   YY    2 digit year
 *   MMMM  full month name (January, February, ...)
 *   M     month number, no leading zero
 *   DD    zero padded day of month
 *   D     day of month, no leading zero
 *   HH    zero padded 24 hour
 *   h     12 hour, no leading zero
 *   mm    zero padded minutes
 *   ss    zero padded seconds
 *   A     AM or PM
 *
 * Anything else is copied through as written.
 *
 * This is the only place in the project that needs a time zone database.
 * The library itself does not, which is the whole point of taking a plain
 * calendar date in and handing UTC instants back. Named zones stay here,
 * in test code, where a tzdb is a reasonable thing to require.
 *
 * @throws std::runtime_error if the instant is absent.
 */
std::string formatInZone(
    const OptInstant &time, const std::string &tzName,
    const std::string &formatStr);

#endif // MOMENTFORMAT_HPP
