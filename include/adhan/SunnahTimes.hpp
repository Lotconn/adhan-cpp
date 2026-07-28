#ifndef SUNNAHTIMES_HPP
#define SUNNAHTIMES_HPP

#include "JSDate.hpp"
#include "PrayerTimes.hpp"

namespace Adhan {

class SunnahTimes {
public:
  JSDate middleOfTheNight;
  JSDate lastThirdOfTheNight;

  SunnahTimes(const PrayerTimes &prayerTimes);
};
} // namespace Adhan

#endif /* SUNNAHTIMES_HPP */