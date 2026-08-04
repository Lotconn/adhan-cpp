#include <adhan/DateUtils.hpp>
#include <adhan/SunnahTimes.hpp>

namespace Adhan {

SunnahTimes::SunnahTimes(const PrayerTimes &prayerTimes) {
  auto date = prayerTimes.date;
  auto nextDay = dateByAddingDays(date, 1);
  auto nextDayPrayerTimes = PrayerTimes(
      prayerTimes.coordinates, nextDay, prayerTimes.calculationParameters);

  auto nightDuration =
      static_cast<double>(
          nextDayPrayerTimes.fajr.getTime() - prayerTimes.maghrib.getTime()) /
      1000.0;

  this->middleOfTheNight = roundedMinute(
      dateByAddingSeconds(prayerTimes.maghrib, nightDuration / 2.0));

  this->lastThirdOfTheNight = roundedMinute(
      dateByAddingSeconds(prayerTimes.maghrib, nightDuration * (2.0 / 3.0)));
}

} // namespace Adhan