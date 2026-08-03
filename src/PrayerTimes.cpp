#include <adhan/Astronomical.hpp>
#include <adhan/DateUtils.hpp>
#include <adhan/Madhab.hpp>
#include <adhan/PolarCircleResolution.hpp>
#include <adhan/PrayerTimes.hpp>
#include <adhan/SolarTime.hpp>
#include <adhan/TimeComponents.hpp>

#include <cmath>
#include <limits>

namespace Adhan {

namespace {
/**
 * Mirrors JS's `Number(someDate)` on an Invalid Date evaluating to NaN,
 * without tripping DateTime::getTime()'s assert on an invalid input.
 */
double millisOrNaN(const DateTime &d) {
  if (!d.isValid()) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return static_cast<double>(d.getTime());
}
} // namespace

PrayerTimes::PrayerTimes(
    const Coordinates &coordinates, const DateTime &date,
    const CalculationParameters &calculationParameters)
    : coordinates(coordinates), date(date),
      calculationParameters(calculationParameters) {
  SolarTime solarTime(date, coordinates);

  DateTime fajrTime = DateTime::invalid();
  DateTime sunriseTime = DateTime::invalid();
  DateTime dhuhrTime = DateTime::invalid();
  DateTime asrTime = DateTime::invalid();
  DateTime sunsetTime = DateTime::invalid();
  DateTime maghribTime = DateTime::invalid();
  DateTime ishaTime = DateTime::invalid();

  double nightFraction = 0;

  dhuhrTime = TimeComponents(solarTime.transit)
                  .utcDate(date.getFullYear(), date.getMonth(), date.getDate());
  sunriseTime =
      TimeComponents(solarTime.sunrise)
          .utcDate(date.getFullYear(), date.getMonth(), date.getDate());
  sunsetTime =
      TimeComponents(solarTime.sunset)
          .utcDate(date.getFullYear(), date.getMonth(), date.getDate());

  DateTime tomorrow = dateByAddingDays(date, 1);
  SolarTime tomorrowSolarTime(tomorrow, coordinates);

  PolarCircleResolution polarCircleResolver =
      calculationParameters.polarCircleResolution;
  if ((!isValidDate(sunriseTime) || !isValidDate(sunsetTime) ||
       std::isnan(tomorrowSolarTime.sunrise)) &&
      polarCircleResolver != PolarCircleResolution::Unresolved) {
    PolarCircleResolver resolved =
        polarCircleResolvedValues(polarCircleResolver, date, coordinates);
    solarTime = resolved.solarTime;
    tomorrowSolarTime = resolved.tomorrowSolarTime;

    dhuhrTime =
        TimeComponents(solarTime.transit)
            .utcDate(date.getFullYear(), date.getMonth(), date.getDate());
    sunriseTime =
        TimeComponents(solarTime.sunrise)
            .utcDate(date.getFullYear(), date.getMonth(), date.getDate());
    sunsetTime =
        TimeComponents(solarTime.sunset)
            .utcDate(date.getFullYear(), date.getMonth(), date.getDate());
  }

  asrTime = TimeComponents(solarTime.afternoon(
                               shadow_length(calculationParameters.madhab)))
                .utcDate(date.getFullYear(), date.getMonth(), date.getDate());

  DateTime tomorrowSunrise =
      TimeComponents(tomorrowSolarTime.sunrise)
          .utcDate(
              tomorrow.getFullYear(), tomorrow.getMonth(), tomorrow.getDate());
  double night =
      (millisOrNaN(tomorrowSunrise) - millisOrNaN(sunsetTime)) / 1000;

  fajrTime = TimeComponents(solarTime.hourAngle(
                                -1 * calculationParameters.fajrAngle, false))
                 .utcDate(date.getFullYear(), date.getMonth(), date.getDate());

  // special case for moonsighting committee above latitude 55
  if (calculationParameters.method == "MoonsightingCommittee" &&
      coordinates.latitude >= 55) {
    nightFraction = night / 7;
    fajrTime = dateByAddingSeconds(sunriseTime, -nightFraction);
  }

  DateTime safeFajr = [&]() {
    if (calculationParameters.method == "MoonsightingCommittee") {
      return Astronomical::seasonAdjustedMorningTwilight(
          coordinates.latitude, dayOfYear(date), date.getFullYear(),
          sunriseTime);
    } else {
      double portion = calculationParameters.nightPortions().fajr;
      nightFraction = portion * night;
      return dateByAddingSeconds(sunriseTime, -nightFraction);
    }
  }();

  if (!fajrTime.isValid() || safeFajr > fajrTime) {
    fajrTime = safeFajr;
  }

  if (calculationParameters.ishaInterval > 0) {
    ishaTime =
        dateByAddingMinutes(sunsetTime, calculationParameters.ishaInterval);
  } else {
    ishaTime =
        TimeComponents(
            solarTime.hourAngle(-1 * calculationParameters.ishaAngle, true))
            .utcDate(date.getFullYear(), date.getMonth(), date.getDate());

    /* special case for moonsighting committee above latitude 55 */
    if (calculationParameters.method == "MoonsightingCommittee" &&
        coordinates.latitude >= 55) {
      nightFraction = night / 7;
      ishaTime = dateByAddingSeconds(sunsetTime, nightFraction);
    }

    DateTime safeIsha = [&]() {
      if (calculationParameters.method == "MoonsightingCommittee") {
        return Astronomical::seasonAdjustedEveningTwilight(
            coordinates.latitude, dayOfYear(date), date.getFullYear(),
            sunsetTime, calculationParameters.shafaq);
      } else {
        double portion = calculationParameters.nightPortions().isha;
        nightFraction = portion * night;
        return dateByAddingSeconds(sunsetTime, nightFraction);
      }
    }();

    if (!ishaTime.isValid() || safeIsha < ishaTime) {
      ishaTime = safeIsha;
    }
  }

  maghribTime = sunsetTime;
  if (calculationParameters.maghribAngle != 0) {
    DateTime angleBasedMaghrib =
        TimeComponents(
            solarTime.hourAngle(-1 * calculationParameters.maghribAngle, true))
            .utcDate(date.getFullYear(), date.getMonth(), date.getDate());
    if (sunsetTime < angleBasedMaghrib && ishaTime > angleBasedMaghrib) {
      maghribTime = angleBasedMaghrib;
    }
  }

  int fajrAdjustment = calculationParameters.adjustments.fajr +
                       calculationParameters.methodAdjustments.fajr;
  int sunriseAdjustment = calculationParameters.adjustments.sunrise +
                          calculationParameters.methodAdjustments.sunrise;
  int dhuhrAdjustment = calculationParameters.adjustments.dhuhr +
                        calculationParameters.methodAdjustments.dhuhr;
  int asrAdjustment = calculationParameters.adjustments.asr +
                      calculationParameters.methodAdjustments.asr;
  int maghribAdjustment = calculationParameters.adjustments.maghrib +
                          calculationParameters.methodAdjustments.maghrib;
  int ishaAdjustment = calculationParameters.adjustments.isha +
                       calculationParameters.methodAdjustments.isha;

  fajr = roundedMinute(
      dateByAddingMinutes(fajrTime, fajrAdjustment),
      calculationParameters.rounding);
  sunrise = roundedMinute(
      dateByAddingMinutes(sunriseTime, sunriseAdjustment),
      calculationParameters.rounding);
  dhuhr = roundedMinute(
      dateByAddingMinutes(dhuhrTime, dhuhrAdjustment),
      calculationParameters.rounding);
  asr = roundedMinute(
      dateByAddingMinutes(asrTime, asrAdjustment),
      calculationParameters.rounding);
  sunset = roundedMinute(sunsetTime, calculationParameters.rounding);
  maghrib = roundedMinute(
      dateByAddingMinutes(maghribTime, maghribAdjustment),
      calculationParameters.rounding);
  isha = roundedMinute(
      dateByAddingMinutes(ishaTime, ishaAdjustment),
      calculationParameters.rounding);
}

std::optional<DateTime> PrayerTimes::timeForPrayer(Prayer prayer) const {
  if (prayer == Prayer::Fajr) {
    return fajr;
  } else if (prayer == Prayer::Sunrise) {
    return sunrise;
  } else if (prayer == Prayer::Dhuhr) {
    return dhuhr;
  } else if (prayer == Prayer::Asr) {
    return asr;
  } else if (prayer == Prayer::Maghrib) {
    return maghrib;
  } else if (prayer == Prayer::Isha) {
    return isha;
  } else {
    return std::nullopt;
  }
}

Prayer PrayerTimes::currentPrayer(const DateTime &date) const {
  if (date >= isha) {
    return Prayer::Isha;
  } else if (date >= maghrib) {
    return Prayer::Maghrib;
  } else if (date >= asr) {
    return Prayer::Asr;
  } else if (date >= dhuhr) {
    return Prayer::Dhuhr;
  } else if (date >= sunrise) {
    return Prayer::Sunrise;
  } else if (date >= fajr) {
    return Prayer::Fajr;
  } else {
    return Prayer::None;
  }
}

Prayer PrayerTimes::nextPrayer(const DateTime &date) const {
  if (date >= isha) {
    return Prayer::None;
  } else if (date >= maghrib) {
    return Prayer::Isha;
  } else if (date >= asr) {
    return Prayer::Maghrib;
  } else if (date >= dhuhr) {
    return Prayer::Asr;
  } else if (date >= sunrise) {
    return Prayer::Dhuhr;
  } else if (date >= fajr) {
    return Prayer::Sunrise;
  } else {
    return Prayer::Fajr;
  }
}
} // namespace Adhan