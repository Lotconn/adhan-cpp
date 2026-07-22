#ifndef PRAYERTIMES_HPP
#define PRAYERTIMES_HPP

#include "CalculationParameters.hpp"
#include "Coordinates.hpp"
#include "JSDate.hpp"
#include "Prayer.hpp"

#include <optional>

class PrayerTimes {
public:
  JSDate fajr;
  JSDate sunrise;
  JSDate dhuhr;
  JSDate asr;
  JSDate sunset;
  JSDate maghrib;
  JSDate isha;

  Coordinates coordinates;
  JSDate date;
  CalculationParameters calculationParameters;

  PrayerTimes(const Coordinates &coordinates, const JSDate &date,
              const CalculationParameters &calculationParameters);

  std::optional<JSDate> timeForPrayer(Prayer prayer) const;
  Prayer currentPrayer(const JSDate &date = JSDate::now()) const;
  Prayer nextPrayer(const JSDate &date = JSDate::now()) const;
};

#endif // PRAYERTIMES_HPP