#ifndef POLARCIRCLERESOLUTION_HPP
#define POLARCIRCLERESOLUTION_HPP

#include "Coordinates.hpp"
#include "JSDate.hpp"
#include "SolarTime.hpp"

enum class PolarCircleResolution {
  AqrabBalad,
  AqrabYaum,
  Unresolved,
};

struct PolarCircleResolver {
  JSDate date;
  JSDate tomorrow;
  Coordinates coordinates;
  SolarTime solarTime;
  SolarTime tomorrowSolarTime;
};

PolarCircleResolver polarCircleResolvedValues(PolarCircleResolution resolver,
                                              const JSDate &date,
                                              const Coordinates &coordinates);

#endif // POLARCIRCLERESOLUTION_HPP