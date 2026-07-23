#include <Astronomical.hpp>
#include <MathUtils.hpp>
#include <SolarTime.hpp>

#include <cmath>

namespace adhan {

SolarTime::SolarTime(const JSDate &date, const Coordinates &coordinates)
    : observer(coordinates),
      solar(Astronomical::julianDay(date.getFullYear(), date.getMonth() + 1,
                                    date.getDate(), 0)),
      prevSolar(Astronomical::julianDay(date.getFullYear(), date.getMonth() + 1,
                                        date.getDate(), 0) -
                1),
      nextSolar(Astronomical::julianDay(date.getFullYear(), date.getMonth() + 1,
                                        date.getDate(), 0) +
                1) {
  const double m0 = Astronomical::approximateTransit(
      coordinates.longitude, solar.apparentSiderealTime, solar.rightAscension);
  const double solarAltitude = -50.0 / 60.0;

  approxTransit = m0;

  transit = Astronomical::correctedTransit(
      m0, coordinates.longitude, solar.apparentSiderealTime,
      solar.rightAscension, prevSolar.rightAscension, nextSolar.rightAscension);

  sunrise = Astronomical::correctedHourAngle(
      m0, solarAltitude, coordinates, false, solar.apparentSiderealTime,
      solar.rightAscension, prevSolar.rightAscension, nextSolar.rightAscension,
      solar.declination, prevSolar.declination, nextSolar.declination);

  sunset = Astronomical::correctedHourAngle(
      m0, solarAltitude, coordinates, true, solar.apparentSiderealTime,
      solar.rightAscension, prevSolar.rightAscension, nextSolar.rightAscension,
      solar.declination, prevSolar.declination, nextSolar.declination);
}

double SolarTime::hourAngle(double angle, bool afterTransit) {
  return Astronomical::correctedHourAngle(
      approxTransit, angle, observer, afterTransit, solar.apparentSiderealTime,
      solar.rightAscension, prevSolar.rightAscension, nextSolar.rightAscension,
      solar.declination, prevSolar.declination, nextSolar.declination);
}

double SolarTime::afternoon(double shadowLength) {
  // (UPSTREAM) TODO source shadow angle calculation
  const double tangent = std::abs(observer.latitude - solar.declination);
  const double inverse = shadowLength + std::tan(degreesToRadians(tangent));
  const double angle = radiansToDegrees(std::atan(1.0 / inverse));
  return hourAngle(angle, true);
}
} // namespace adhan