#ifndef SOLARTIME_HPP
#define SOLARTIME_HPP

#include "Coordinates.hpp"
#include "DateTime.hpp"
#include "SolarCoordinates.hpp"

namespace Adhan {

class SolarTime {
public:
  Coordinates observer;
  SolarCoordinates solar;
  SolarCoordinates prevSolar;
  SolarCoordinates nextSolar;
  double approxTransit;
  double transit;
  double sunrise;
  double sunset;

  SolarTime(const DateTime &date, const Coordinates &coordinates);

  double hourAngle(double angle, bool afterTransit);
  double afternoon(double shadowLength);
};
} // namespace Adhan

#endif // SOLARTIME_HPP