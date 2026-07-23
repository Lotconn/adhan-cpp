#include <DateUtils.hpp>
#include <PolarCircleResolution.hpp>

#include <cmath>

namespace {

/* Degrees to add/remove at each resolution step */
const double LATITUDE_VARIATION_STEP = 0.5;

/* Based on https://en.wikipedia.org/wiki/Midnight_sun */
const double UNSAFE_LATITUDE = 65;

bool isValidSolarTime(const SolarTime &solarTime) {
  return !std::isnan(solarTime.sunrise) && !std::isnan(solarTime.sunset);
}

/**
 * Mirrors JS's Math.sign:
 * -1, 0, or 1 (unlike std::copysign, which treats 0 as positive).
 */
double jsSign(double x) {
  if (x > 0)
    return 1;
  if (x < 0)
    return -1;
  return 0;
}

std::optional<PolarCircleResolver>
aqrabYaumResolver(const Coordinates &coordinates, const JSDate &date,
                  int daysAdded = 1, int direction = 1) {
  if (daysAdded > static_cast<int>(std::ceil(365 / 2.0))) {
    return std::nullopt;
  }

  const JSDate testDate = dateByAddingDays(date, direction * daysAdded);
  const JSDate tomorrow = dateByAddingDays(testDate, 1);
  SolarTime solarTime(testDate, coordinates);
  SolarTime tomorrowSolarTime(tomorrow, coordinates);

  if (!isValidSolarTime(solarTime) || !isValidSolarTime(tomorrowSolarTime)) {
    return aqrabYaumResolver(coordinates, date,
                             daysAdded + (direction > 0 ? 0 : 1), -direction);
  }

  return PolarCircleResolver{
      date, tomorrow, coordinates, solarTime, tomorrowSolarTime,
  };
}

std::optional<PolarCircleResolver>
aqrabBaladResolver(const Coordinates &coordinates, const JSDate &date,
                   double latitude) {
  const Coordinates adjusted(latitude, coordinates.longitude);
  SolarTime solarTime(date, adjusted);
  const JSDate tomorrow = dateByAddingDays(date, 1);
  SolarTime tomorrowSolarTime(tomorrow, adjusted);

  if (!isValidSolarTime(solarTime) || !isValidSolarTime(tomorrowSolarTime)) {
    if (std::abs(latitude) >= UNSAFE_LATITUDE) {
      return aqrabBaladResolver(coordinates, date,
                                latitude -
                                    jsSign(latitude) * LATITUDE_VARIATION_STEP);
    }
    return std::nullopt;
  }

  return PolarCircleResolver{
      date,
      tomorrow,
      Coordinates(latitude, coordinates.longitude),
      solarTime,
      tomorrowSolarTime,
  };
}

} // namespace

PolarCircleResolver polarCircleResolvedValues(PolarCircleResolution resolver,
                                              const JSDate &date,
                                              const Coordinates &coordinates) {

#ifdef ADHAN_TESTING
  ++polarCircleResolvedValuesCallCount;
#endif

  auto makeDefault = [&]() {
    const JSDate tomorrow = dateByAddingDays(date, 1);
    return PolarCircleResolver{
        date,
        tomorrow,
        coordinates,
        SolarTime(date, coordinates),
        SolarTime(tomorrow, coordinates),
    };
  };

  switch (resolver) {
  case PolarCircleResolution::AqrabYaum: {
    auto result = aqrabYaumResolver(coordinates, date);
    return result ? *result : makeDefault();
  }
  case PolarCircleResolution::AqrabBalad: {
    const double latitude = coordinates.latitude;
    auto result = aqrabBaladResolver(coordinates, date,
                                     latitude - jsSign(latitude) *
                                                    LATITUDE_VARIATION_STEP);
    return result ? *result : makeDefault();
  }
  default: {
    return makeDefault();
  }
  }
}