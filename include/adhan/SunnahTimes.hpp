#ifndef SUNNAHTIMES_HPP
#define SUNNAHTIMES_HPP

#include "DateTime.hpp"
#include "PrayerTimes.hpp"

namespace Adhan {

class SunnahTimes {
public:
  DateTime middleOfTheNight;
  DateTime lastThirdOfTheNight;

  SunnahTimes(const PrayerTimes &prayerTimes);
};
} // namespace Adhan

#endif /* SUNNAHTIMES_HPP */