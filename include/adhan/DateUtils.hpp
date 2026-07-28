#ifndef DATEUTILS_HPP
#define DATEUTILS_HPP

#include "JSDate.hpp"
#include "Rounding.hpp"

namespace Adhan {

JSDate dateByAddingDays(const JSDate &date, int days);
JSDate dateByAddingMinutes(const JSDate &date, double minutes);
JSDate dateByAddingSeconds(const JSDate &date, double seconds);
JSDate roundedMinute(const JSDate &date, Rounding rounding = Rounding::Nearest);
bool isLeapYear(int year);
int dayOfYear(const JSDate &date);
bool isValidDate(const JSDate &date);
} // namespace Adhan

#endif // DATEUTILS_HPP