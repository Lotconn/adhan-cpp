#ifndef POLARCIRCLERESOLUTION_HPP
#define POLARCIRCLERESOLUTION_HPP

#include "Coordinates.hpp"
#include "JSDate.hpp"
#include "SolarTime.hpp"

#ifdef ADHAN_TESTING
// Test-only call counter, standing in for vi.spyOn's call-tracking in the
// TS test suite. Compiled out entirely in release builds - no footprint,
// no symbol, in the shipped library.
inline int polarCircleResolvedValuesCallCount = 0;
#endif

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