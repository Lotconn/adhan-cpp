/**
 * LanguagePortQuirks_test.cpp
 *
 * adhan-js has no equivalent of these checks: JavaScript silently produces
 * NaN/Infinity/garbage Date objects instead of throwing. These tests cover
 * validation that only exists because of this C++ port, guarding against
 * undefined behavior (e.g. casting a non-finite double to int) and against
 * physically-nonsensical input (e.g. a latitude of 200 degrees) that the
 * original JS implementation simply lets through uncaught.
 */

#include "doctest.h"

#include <adhan/Astronomical.hpp>
#include <adhan/CalculationParameters.hpp>
#include <adhan/Coordinates.hpp>
#include <adhan/DateTime.hpp>
#include <adhan/SolarCoordinates.hpp>
#include <adhan/TimeComponents.hpp>

#include <limits>
#include <optional>
#include <stdexcept>

using namespace Adhan;

TEST_SUITE("LanguagePortQuirks") {

  TEST_CASE("Coordinates rejects out-of-range latitude") {
    CHECK_THROWS_AS(Coordinates(90.0001, 0.0), std::invalid_argument);
    CHECK_THROWS_AS(Coordinates(-90.0001, 0.0), std::invalid_argument);
    CHECK_NOTHROW(Coordinates(90.0, 0.0));
    CHECK_NOTHROW(Coordinates(-90.0, 0.0));
  }

  TEST_CASE("Coordinates rejects out-of-range longitude") {
    CHECK_THROWS_AS(Coordinates(0.0, 180.0001), std::invalid_argument);
    CHECK_THROWS_AS(Coordinates(0.0, -180.0001), std::invalid_argument);
    CHECK_NOTHROW(Coordinates(0.0, 180.0));
    CHECK_NOTHROW(Coordinates(0.0, -180.0));
  }

  TEST_CASE("DateTime rejects a year outside the representable range") {
    // std::chrono::year is only guaranteed valid in [-32767, 32767].
    CHECK_THROWS_AS(DateTime(32768, 0, 1), std::invalid_argument);
    CHECK_THROWS_AS(DateTime(-32768, 0, 1), std::invalid_argument);
    CHECK_NOTHROW(DateTime(32767, 0, 1));
    CHECK_NOTHROW(DateTime(-32767, 0, 1));
  }

  TEST_CASE("CalculationParameters rejects out-of-range fajrAngle") {
    CHECK_THROWS_AS(
        CalculationParameters(std::nullopt, 90.0001, 0, 0, 0),
        std::invalid_argument);
    CHECK_THROWS_AS(
        CalculationParameters(std::nullopt, -0.0001, 0, 0, 0),
        std::invalid_argument);
    CHECK_NOTHROW(CalculationParameters(std::nullopt, 90.0, 0, 0, 0));
    CHECK_NOTHROW(CalculationParameters(std::nullopt, 0.0, 0, 0, 0));
  }

  TEST_CASE("CalculationParameters rejects out-of-range ishaAngle") {
    CHECK_THROWS_AS(
        CalculationParameters(std::nullopt, 0, 90.0001, 0, 0),
        std::invalid_argument);
    CHECK_THROWS_AS(
        CalculationParameters(std::nullopt, 0, -0.0001, 0, 0),
        std::invalid_argument);
    CHECK_NOTHROW(CalculationParameters(std::nullopt, 0, 90.0, 0, 0));
    CHECK_NOTHROW(CalculationParameters(std::nullopt, 0, 0.0, 0, 0));
  }

  TEST_CASE("CalculationParameters rejects a negative ishaInterval") {
    CHECK_THROWS_AS(
        CalculationParameters(std::nullopt, 0, 0, -0.0001, 0),
        std::invalid_argument);
    CHECK_NOTHROW(CalculationParameters(std::nullopt, 0, 0, 0.0, 0));
  }

  TEST_CASE("CalculationParameters rejects out-of-range maghribAngle") {
    CHECK_THROWS_AS(
        CalculationParameters(std::nullopt, 0, 0, 0, 90.0001),
        std::invalid_argument);
    CHECK_THROWS_AS(
        CalculationParameters(std::nullopt, 0, 0, 0, -0.0001),
        std::invalid_argument);
    CHECK_NOTHROW(CalculationParameters(std::nullopt, 0, 0, 0, 90.0));
    CHECK_NOTHROW(CalculationParameters(std::nullopt, 0, 0, 0, 0.0));
  }

  TEST_CASE("SolarCoordinates rejects a non-finite julianDay") {
    double inf = std::numeric_limits<double>::infinity();
    double qNaN = std::numeric_limits<double>::quiet_NaN();
    CHECK_THROWS_AS(SolarCoordinates{inf}, std::invalid_argument);
    CHECK_THROWS_AS(SolarCoordinates{-inf}, std::invalid_argument);
    CHECK_THROWS_AS(SolarCoordinates{qNaN}, std::invalid_argument);
    CHECK_NOTHROW(SolarCoordinates(2451545.0)); // J2000.0 epoch, a normal JD
  }

  TEST_CASE("Astronomical::julianDay rejects an out-of-range month") {
    CHECK_THROWS_AS(
        Astronomical::julianDay(2026, 0, 1, 0), std::invalid_argument);
    CHECK_THROWS_AS(
        Astronomical::julianDay(2026, 13, 1, 0), std::invalid_argument);
    CHECK_NOTHROW(Astronomical::julianDay(2026, 1, 1, 0));
    CHECK_NOTHROW(Astronomical::julianDay(2026, 12, 1, 0));
  }

  TEST_CASE("Astronomical::julianDay rejects an out-of-range day") {
    CHECK_THROWS_AS(
        Astronomical::julianDay(2026, 1, 0, 0), std::invalid_argument);
    CHECK_THROWS_AS(
        Astronomical::julianDay(2026, 1, 32, 0), std::invalid_argument);
    CHECK_NOTHROW(Astronomical::julianDay(2026, 1, 1, 0));
    CHECK_NOTHROW(Astronomical::julianDay(2026, 1, 31, 0));
  }

  TEST_CASE(
      "Astronomical::julianDay leaves hours unbounded by design, "
      "so overflow rolls into the next day instead of throwing") {
    const double base = Astronomical::julianDay(2010, 1, 1, 0);
    const double rolled = Astronomical::julianDay(2010, 1, 1, 48);
    CHECK(rolled == doctest::Approx(base + 2));
  }

  TEST_CASE("TimeComponents treats non-finite input as invalid") {
    CHECK_FALSE(
        TimeComponents(std::numeric_limits<double>::quiet_NaN()).isValid());
    CHECK_FALSE(
        TimeComponents(std::numeric_limits<double>::infinity()).isValid());
    CHECK_FALSE(
        TimeComponents(-std::numeric_limits<double>::infinity()).isValid());
    CHECK(TimeComponents(13.5).isValid()); // 13:30:00
  }

} // TEST_SUITE("LanguagePortQuirks")