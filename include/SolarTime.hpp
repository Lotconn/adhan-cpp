#ifndef SOLARTIME_HPP
#define SOLARTIME_HPP

#include "Coordinates.hpp"
#include "JSDate.hpp"
#include "SolarCoordinates.hpp"

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

  SolarTime(const JSDate &date, const Coordinates &coordinates);

  double hourAngle(double angle, bool afterTransit);
  double afternoon(double shadowLength);
};

#endif // SOLARTIME_HPP