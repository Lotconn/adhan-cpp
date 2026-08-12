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
#include <adhan/DateUtils.hpp>
#include <adhan/SolarCoordinates.hpp>
#include <adhan/TimeComponents.hpp>

#include <chrono>
#include <cmath>
#include <limits>
#include <optional>
#include <stdexcept>

using namespace Adhan;
using namespace std::chrono;

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

  /**
   * Documents a contract rather than guarding our own code. year_month_day
   * is the standard library's, and it is not going to change under us. It
   * earns a place here because it records where the port stopped behaving
   * like the original: JS rolls February 30th over to March 2nd without a
   * word, and we hand the caller a date that says it is not a real day.
   */
  TEST_CASE("year_month_day reports impossible calendar dates") {
    CHECK_FALSE((2026y / February / 30d).ok());
    CHECK_FALSE((2025y / February / 29d).ok());
    CHECK((2024y / February / 29d).ok());
    CHECK((2026y / January / 31d).ok());
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

  TEST_CASE("dateByAddingSeconds refuses a non-finite shift") {
    /**
     * The polar paths can hand this function a NaN, because the night
     * length is NaN whenever sunset or the next sunrise is missing.
     * Casting that to an integer would be undefined behaviour, so it comes
     * back absent instead. JS just builds an Invalid Date.
     */
    const double qNaN = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    const Instant epoch{milliseconds{0}};

    CHECK_FALSE(dateByAddingSeconds(epoch, qNaN).has_value());
    CHECK_FALSE(dateByAddingSeconds(epoch, inf).has_value());
    CHECK_FALSE(dateByAddingSeconds(epoch, -inf).has_value());
    CHECK(dateByAddingSeconds(epoch, 60).has_value());
  }

  TEST_CASE("dateByAddingSeconds refuses a shift that leaves the range") {
    /**
     * 8.64e15 milliseconds either side of 1970 is as far as a JS Date
     * goes, and it is also the last point a double still counts whole
     * milliseconds exactly. Past it the answer would be quietly wrong.
     */
    const Instant epoch{milliseconds{0}};

    CHECK(dateByAddingSeconds(epoch, 8.64e12).has_value());
    CHECK_FALSE(dateByAddingSeconds(epoch, 8.64e12 + 1).has_value());
    CHECK_FALSE(dateByAddingSeconds(epoch, -(8.64e12 + 1)).has_value());
  }

  TEST_CASE("an absent time spreads instead of being treated as the earliest") {
    /**
     * This is the trap the port has to keep clear of. Comparing an Invalid
     * Date in JS is false whichever way round you ask. std::optional says
     * nullopt is smaller than any value, so leaning on its operators would
     * pick the wrong branch. The library never compares optionals directly,
     * and neither should anything built on it.
     */
    const OptInstant absent = std::nullopt;
    const OptInstant present = Instant{milliseconds{0}};

    /**
     * Not our behaviour to fix, this is what std::optional promises. It is
     * written down so the next person reads it before reaching for `>` on a
     * pair of these.
     */
    CHECK(absent < present);
    CHECK_FALSE(dateByAddingSeconds(absent, 60).has_value());
    CHECK_FALSE(roundedMinute(absent).has_value());
    CHECK(std::isnan(secondsBetween(absent, present)));
    CHECK(std::isnan(secondsBetween(present, absent)));
    CHECK(secondsBetween(present, present) == 0.0);
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

  TEST_CASE("TimeComponents accepts hours outside a single day") {
    /**
     * hourAngle can return something below 0 or above 24. Those are not
     * mistakes, they mean the time falls on the day either side, and the
     * duration arithmetic in utcDate carries them over on its own.
     */
    const year_month_day date{2015y / July / 12d};

    const auto rolledBack = TimeComponents(-1.5).utcDate(date);
    REQUIRE(rolledBack.has_value());
    CHECK(*rolledBack == sys_days{2015y / July / 11d} + 22h + 30min);

    const auto rolledForward = TimeComponents(25.25).utcDate(date);
    REQUIRE(rolledForward.has_value());
    CHECK(*rolledForward == sys_days{2015y / July / 13d} + 1h + 15min);

    CHECK_FALSE(TimeComponents(std::numeric_limits<double>::quiet_NaN())
                    .utcDate(date)
                    .has_value());
  }

} // TEST_SUITE("LanguagePortQuirks")