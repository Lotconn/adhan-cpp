#ifndef SOLARCOORDINATES_HPP
#define SOLARCOORDINATES_HPP

namespace adhan {

class SolarCoordinates {
public:
  double declination;
  double rightAscension;
  double apparentSiderealTime;

  explicit SolarCoordinates(double julianDay);
};
} // namespace adhan

#endif // SOLARCOORDINATES_HPP