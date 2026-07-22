#ifndef SOLARCOORDINATES_HPP
#define SOLARCOORDINATES_HPP

class SolarCoordinates {
public:
  double declination;
  double rightAscension;
  double apparentSiderealTime;

  explicit SolarCoordinates(double julianDay);
};

#endif // SOLARCOORDINATES_HPP