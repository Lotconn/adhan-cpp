#ifndef MOMENTFORMAT_HPP
#define MOMENTFORMAT_HPP

#include "JSDate.hpp"
#include <string>

// Test-only helper. Formats a JSDate as if viewed in the given IANA time zone,
// using a small subset of moment.js format tokens — only the tokens actually
// used by the ported test suite:
//   YYYY  - 4-digit year
//   MMMM  - full month name (January, February, ...)
//   DD    - zero-padded day of month
//   HH    - zero-padded 24-hour
//   h     - 12-hour, no leading zero
//   mm    - zero-padded minutes
//   A     - AM/PM
// Any other character (spaces, colons, commas) is copied through literally.
std::string formatInZone(const JSDate &date, const std::string &tzName,
                         const std::string &formatStr);

#endif // MOMENTFORMAT_HPP