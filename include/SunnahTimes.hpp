#ifndef SUNNAHTIMES_HPP
#define SUNNAHTIMES_HPP

#include <JSDate.hpp>
#include <PrayerTimes.hpp>

class SunnahTimes {
public:
  JSDate middleOfTheNight;
  JSDate lastThirdOfTheNight;

  SunnahTimes(const PrayerTimes &prayerTimes);
};

#endif /* SUNNAHTIMES_HPP */