#ifndef PRAYERTIMES_HPP
#define PRAYERTIMES_HPP

#include "CalculationParameters.hpp"
#include "Coordinates.hpp"
#include "DateTime.hpp"
#include "Prayer.hpp"

#include <optional>

namespace Adhan {

class PrayerTimes {
public:
  DateTime fajr;
  DateTime sunrise;
  DateTime dhuhr;
  DateTime asr;
  DateTime sunset;
  DateTime maghrib;
  DateTime isha;

  Coordinates coordinates;
  DateTime date;
  CalculationParameters calculationParameters;

  PrayerTimes(
      const Coordinates &coordinates, const DateTime &date,
      const CalculationParameters &calculationParameters);

  std::optional<DateTime> timeForPrayer(Prayer prayer) const;
  Prayer currentPrayer(const DateTime &_date = DateTime::now()) const;
  Prayer nextPrayer(const DateTime &_date = DateTime::now()) const;
};
} // namespace Adhan

#endif // PRAYERTIMES_HPP