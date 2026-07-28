#ifndef DATEUTILS_HPP
#define DATEUTILS_HPP

#include "DateTime.hpp"
#include "Rounding.hpp"

namespace Adhan {

DateTime dateByAddingDays(const DateTime &date, int days);
DateTime dateByAddingMinutes(const DateTime &date, double minutes);
DateTime dateByAddingSeconds(const DateTime &date, double seconds);
DateTime roundedMinute(const DateTime &date, Rounding rounding = Rounding::Nearest);
bool isLeapYear(int year);
int dayOfYear(const DateTime &date);
bool isValidDate(const DateTime &date);
} // namespace Adhan

#endif // DATEUTILS_HPP