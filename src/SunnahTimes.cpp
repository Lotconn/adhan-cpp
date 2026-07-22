#include "SunnahTimes.hpp"
#include "DateUtils.hpp"

SunnahTimes::SunnahTimes(PrayerTimes &prayerTimes) {
  auto date = prayerTimes.date;
  auto nextDay = dateByAddingDays(date, 1);
  auto nextDayPrayerTimes = PrayerTimes(prayerTimes.coordinates, nextDay,
                                        prayerTimes.calculationParameters);

  auto nightDuration =
      (nextDayPrayerTimes.fajr.getTime() - prayerTimes.maghrib.getTime()) /
      1000.0;

  this->middleofthenight = roundedMinute(
      dateByAddingSeconds(prayerTimes.maghrib, nightDuration / 2.0));

  this->lastThirdOfTheNight = roundedMinute(
      dateByAddingSeconds(prayerTimes.maghrib, nightDuration * (2.0 / 3.0)));
}