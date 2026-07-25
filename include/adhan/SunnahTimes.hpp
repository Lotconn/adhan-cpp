#ifndef SUNNAHTIMES_HPP
#define SUNNAHTIMES_HPP

#include "JSDate.hpp"
#include "PrayerTimes.hpp"

namespace adhan {

class SunnahTimes {
public:
  JSDate middleOfTheNight;
  JSDate lastThirdOfTheNight;

  SunnahTimes(const PrayerTimes &prayerTimes);
};
} // namespace adhan

#endif /* SUNNAHTIMES_HPP */