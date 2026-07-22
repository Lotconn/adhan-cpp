#ifndef SUNNAHTIMES_HPP
#define SUNNAHTIMES_HPP

#include "JSDate.hpp"
#include "PrayerTimes.hpp"

class SunnahTimes {
public:
  JSDate middleofthenight;
  JSDate lastThirdOfTheNight;

  SunnahTimes(PrayerTimes &prayerTimes);
};

#endif /* SUNNAHTIMES_HPP */