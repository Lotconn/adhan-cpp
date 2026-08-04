#ifndef POLARCIRCLERESOLUTION_HPP
#define POLARCIRCLERESOLUTION_HPP

#include "Coordinates.hpp"
#include "DateTime.hpp"
#include "SolarTime.hpp"
#include <cstdint>

namespace Adhan {

#ifdef ADHAN_TESTING

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
// Test-only call counter, standing in for vi.spyOn's call-tracking in the
// TS test suite. Excluded entirely in release builds
inline int polarCircleResolvedValuesCallCount = 0;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)
#endif

enum class PolarCircleResolution : std::int8_t {
  AqrabBalad,
  AqrabYaum,
  Unresolved,
};

struct PolarCircleResolver {
  DateTime date;
  DateTime tomorrow;
  Coordinates coordinates;
  SolarTime solarTime;
  SolarTime tomorrowSolarTime;
};

PolarCircleResolver polarCircleResolvedValues(
    PolarCircleResolution resolver, const DateTime &date,
    const Coordinates &coordinates);

} // namespace Adhan

#endif // POLARCIRCLERESOLUTION_HPP