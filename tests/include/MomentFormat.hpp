#ifndef MOMENTFORMAT_HPP
#define MOMENTFORMAT_HPP

#include <adhan/DateTime.hpp>
#include <string>

using namespace Adhan;

/**
 * Test-only helper. Formats a DateTime as if viewed in the given IANA time
 * zone, using a small subset of moment.js format tokens — only the tokens
 * actually used by the ported test suite: YYYY  - 4-digit year MMMM  - full
 * month name (January, February, ...) DD    - zero-padded day of month HH    -
 * zero-padded 24-hour h     - 12-hour, no leading zero mm    - zero-padded
 * minutes A     - AM/PM Any other character (spaces, colons, commas) is copied
 * through literally.
 */
std::string formatInZone(
    const DateTime &date, const std::string &tzName,
    const std::string &formatStr);

#endif // MOMENTFORMAT_HPP