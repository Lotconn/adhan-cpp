#include "doctest.h"

#include "Coordinates.hpp"
#include "Qibla.hpp"

#include <cmath>

using namespace adhan;

namespace {
// Mirrors Jest's toBeCloseTo(expected, precision):
// |actual - expected| < 10^-precision / 2
bool closeTo(double actual, double expected, int precision) {
  return std::abs(actual - expected) < std::pow(10, -precision) / 2.0;
}
} // namespace

TEST_CASE("finding Qibla in North America") {
  CHECK(closeTo(qibla(Coordinates(38.9072, -77.0369)), 56.56, 3));
  CHECK(closeTo(qibla(Coordinates(40.7128, -74.0059)), 58.4817, 3));
  CHECK(closeTo(qibla(Coordinates(37.7749, -122.4194)), 18.8438, 3));
  CHECK(closeTo(qibla(Coordinates(61.2181, -149.9003)), 350.883, 3));
}

TEST_CASE("finding Qibla in the South Pacific") {
  CHECK(closeTo(qibla(Coordinates(-33.8688, 151.2093)), 277.4996, 3));
  CHECK(closeTo(qibla(Coordinates(-36.8485, 174.7633)), 261.197, 3));
}

TEST_CASE("finding Qibla in Europe") {
  CHECK(closeTo(qibla(Coordinates(51.5074, -0.1278)), 118.987, 3));
  CHECK(closeTo(qibla(Coordinates(48.8566, 2.3522)), 119.163, 3));
  CHECK(closeTo(qibla(Coordinates(59.9139, 10.7522)), 139.0278, 3));
}

TEST_CASE("finding Qibla in Asia") {
  CHECK(closeTo(qibla(Coordinates(33.7294, 73.0931)), 255.882, 3));
  CHECK(closeTo(qibla(Coordinates(35.6895, 139.6917)), 293.021, 3));
}