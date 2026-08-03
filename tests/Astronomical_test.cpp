#include "doctest.h"

#include <adhan/Astronomical.hpp>
#include <adhan/Coordinates.hpp>
#include <adhan/DateTime.hpp>
#include <adhan/DateUtils.hpp>
#include <adhan/MathUtils.hpp>
#include <adhan/SolarCoordinates.hpp>
#include <adhan/SolarTime.hpp>
#include <adhan/TimeComponents.hpp>

#include <cmath>
#include <string>

using namespace Adhan;

namespace {

// Mirrors Jest's toBeCloseTo(expected, precision):
// |actual - expected| < 10^-precision / 2
bool closeTo(double actual, double expected, int precision) {
  return std::abs(actual - expected) < std::pow(10, -precision) / 2.0;
}

std::string timeString(double hours) {
  TimeComponents comps(hours);
  if (!comps.isValid()) {
    return "";
  }

  // round to the nearest minute
  int minutes =
      comps.minutes + static_cast<int>(std::round(comps.seconds / 60.0));
  std::string minutesStr = std::to_string(minutes);
  if (minutesStr.length() == 1) {
    minutesStr = "0" + minutesStr;
  }
  return std::to_string(comps.hours) + ":" + minutesStr;
}

} // namespace

TEST_CASE("Calculate solar coordinate values") {
  // values from Astronomical Algorithms page 165

  double jd = Astronomical::julianDay(1992, 10, 13);
  SolarCoordinates solar(jd);

  double T = Astronomical::julianCentury(jd);
  double L0 = Astronomical::meanSolarLongitude(T);
  double E0 = Astronomical::meanObliquityOfTheEcliptic(T);
  double Eapp = Astronomical::apparentObliquityOfTheEcliptic(T, E0);
  double M = Astronomical::meanSolarAnomaly(T);
  double C = Astronomical::solarEquationOfTheCenter(T, M);
  double Lambda = Astronomical::apparentSolarLongitude(T, L0);
  double Delta = solar.declination;
  double Alpha = unwindAngle(solar.rightAscension);

  CHECK(closeTo(T, -0.072183436, 9));
  CHECK(closeTo(L0, 201.8072, 4));
  CHECK(closeTo(E0, 23.44023, 4));
  CHECK(closeTo(Eapp, 23.43999, 4));
  CHECK(closeTo(M, 278.99397, 4));
  CHECK(closeTo(C, -1.89732, 4));
  CHECK(closeTo(Lambda, 199.90895, 4));
  CHECK(closeTo(Delta, -7.78507, 4));
  CHECK(closeTo(Alpha, 198.38083, 4));

  // values from Astronomical Algorithms page 88

  jd = Astronomical::julianDay(1987, 4, 10);
  SolarCoordinates solar2(jd);
  T = Astronomical::julianCentury(jd);

  double Theta0 = Astronomical::meanSiderealTime(T);
  double Thetaapp = solar2.apparentSiderealTime;
  double Omega = Astronomical::ascendingLunarNodeLongitude(T);
  E0 = Astronomical::meanObliquityOfTheEcliptic(T);
  L0 = Astronomical::meanSolarLongitude(T);
  double Lp = Astronomical::meanLunarLongitude(T);
  double dPsi = Astronomical::nutationInLongitude(T, L0, Lp, Omega);
  double dE = Astronomical::nutationInObliquity(T, L0, Lp, Omega);
  double E = E0 + dE;

  CHECK(closeTo(Theta0, 197.693195, 5));
  CHECK(closeTo(Thetaapp, 197.6922295833, 3));

  // values from Astronomical Algorithms page 148

  CHECK(closeTo(Omega, 11.2531, 3));
  CHECK(closeTo(dPsi, -0.0010522, 3));
  CHECK(closeTo(dE, 0.0026230556, 4));
  CHECK(closeTo(E0, 23.4409463889, 5));
  CHECK(closeTo(E, 23.4435694444, 4));
}

TEST_CASE("Calculate the Altitude Of Celestial Body") {
  double Phi = 38 + 55.0 / 60 + 17.0 / 3600;
  double Delta = -6 - 43.0 / 60 - 11.61 / 3600;
  double H = 64.352133;
  double altitude = Astronomical::altitudeOfCelestialBody(Phi, Delta, H);
  CHECK(closeTo(altitude, 15.1249, 3));
}

TEST_CASE("Calculate the Transit and Hour Angle") {
  // values from Astronomical Algorithms page 103
  double longitude = -71.0833;
  double Theta = 177.74208;
  double Alpha1 = 40.68021;
  double Alpha2 = 41.73129;
  double Alpha3 = 42.78204;
  double m0 = Astronomical::approximateTransit(longitude, Theta, Alpha2);

  CHECK(closeTo(m0, 0.81965, 4));

  double transit = Astronomical::correctedTransit(
                       m0, longitude, Theta, Alpha2, Alpha1, Alpha3) /
                   24;

  CHECK(closeTo(transit, 0.8198, 4));

  double Delta1 = 18.04761;
  double Delta2 = 18.44092;
  double Delta3 = 18.82742;
  Coordinates coordinates(42.3333, longitude);

  double rise = Astronomical::correctedHourAngle(
                    m0, -0.5667, coordinates, false, Theta, Alpha2, Alpha1,
                    Alpha3, Delta2, Delta1, Delta3) /
                24;
  CHECK(closeTo(rise, 0.51766, 4));
}

TEST_CASE("calculate Solar Time values") {
  // Comparison values generated from
  // http://aa.usno.navy.mil/rstt/onedaytable?form=1&ID=AA&year=2015&month=7&day=12&state=NC&place=raleigh
  Coordinates coordinates(35 + 47.0 / 60, -78 - 39.0 / 60);
  SolarTime solar(DateTime(2015, 6, 12), coordinates);

  double transit = solar.transit;
  double sunrise = solar.sunrise;
  double sunset = solar.sunset;
  double twilightStart = solar.hourAngle(-6, false);
  double twilightEnd = solar.hourAngle(-6, true);
  double invalid = solar.hourAngle(-36, true);
  CHECK(timeString(twilightStart) == "9:38");
  CHECK(timeString(sunrise) == "10:08");
  CHECK(timeString(transit) == "17:20");
  CHECK(timeString(sunset) == "24:32");
  CHECK(timeString(twilightEnd) == "25:02");
  CHECK(timeString(invalid) == "");
}

TEST_CASE("verify Right Ascension Edge Case") {
  Coordinates coordinates(35 + 47.0 / 60, -78 - 39.0 / 60);
  std::vector<SolarTime> solar;
  for (int i = 0; i <= 365; i++) {
    solar.emplace_back(DateTime(2016, 0, i), coordinates);
  }

  for (size_t i = 1; i < solar.size(); i++) {
    const SolarTime &time = solar[i];
    const SolarTime &previousTime = solar[i - 1];
    CHECK(std::abs(time.transit - previousTime.transit) < 1.0 / 60);
    CHECK(std::abs(time.sunrise - previousTime.sunrise) < 2.0 / 60);
    CHECK(std::abs(time.sunset - previousTime.sunset) < 2.0 / 60);
  }
}

TEST_CASE("verify the correct calendar date is being used for calculations") {
  // generated from http://aa.usno.navy.mil/data/docs/RS_OneYear.php for
  // KUKUIHAELE, HAWAII
  Coordinates coordinates(20 + 7.0 / 60, -155 - 34.0 / 60);
  SolarTime day1solar(DateTime(2015, 3, 2), coordinates);
  SolarTime day2solar(DateTime(2015, 3, 3), coordinates);

  double day1 = day1solar.sunrise;
  double day2 = day2solar.sunrise;

  CHECK(timeString(day1) == "16:15");
  CHECK(timeString(day2) == "16:14");
}

TEST_CASE(
    "interpolate a value given previous and next values along with an "
    "interpolation factor") {
  double interpolatedValue =
      Astronomical::interpolate(0.877366, 0.884226, 0.870531, 4.35 / 24);
  CHECK(closeTo(interpolatedValue, 0.876125, 5));

  double i1 = Astronomical::interpolate(1, -1, 3, 0.6);
  CHECK(closeTo(i1, 2.2, 5));

  double i2 = Astronomical::interpolateAngles(1, -1, 3, 0.6);
  CHECK(closeTo(i2, 2.2, 5));

  double i3 = Astronomical::interpolateAngles(1, 359, 3, 0.6);
  CHECK(closeTo(i3, 2.2, 5));
}

TEST_CASE("calculate the Julian day for a given Gregorian date") {
  // Comparison values generated from
  // http://aa.usno.navy.mil/data/docs/JulianDate.php
  CHECK(Astronomical::julianDay(2010, 1, 2) == doctest::Approx(2455198.5));
  CHECK(Astronomical::julianDay(2011, 2, 4) == doctest::Approx(2455596.5));
  CHECK(Astronomical::julianDay(2012, 3, 6) == doctest::Approx(2455992.5));
  CHECK(Astronomical::julianDay(2013, 4, 8) == doctest::Approx(2456390.5));
  CHECK(Astronomical::julianDay(2014, 5, 10) == doctest::Approx(2456787.5));
  CHECK(Astronomical::julianDay(2015, 6, 12) == doctest::Approx(2457185.5));
  CHECK(Astronomical::julianDay(2016, 7, 14) == doctest::Approx(2457583.5));
  CHECK(Astronomical::julianDay(2017, 8, 16) == doctest::Approx(2457981.5));
  CHECK(Astronomical::julianDay(2018, 9, 18) == doctest::Approx(2458379.5));
  CHECK(Astronomical::julianDay(2019, 10, 20) == doctest::Approx(2458776.5));
  CHECK(Astronomical::julianDay(2020, 11, 22) == doctest::Approx(2459175.5));
  CHECK(Astronomical::julianDay(2021, 12, 24) == doctest::Approx(2459572.5));

  double jdVal = 2457215.67708333;
  CHECK(closeTo(Astronomical::julianDay(2015, 7, 12, 4.25), jdVal, 5));

  CHECK(closeTo(Astronomical::julianDay(2015, 7, 12, 8.0), 2457215.833333, 5));
  CHECK(closeTo(Astronomical::julianDay(1992, 10, 13, 0.0), 2448908.5, 5));

  double j1 = Astronomical::julianDay(2010, 1, 3);
  double j2 = Astronomical::julianDay(2010, 1, 1, 48);
  CHECK(j1 == doctest::Approx(j2));
}

TEST_CASE("get the day of the year for a date") {
  CHECK(dayOfYear(DateTime(2015, 0, 1)) == 1);
  CHECK(dayOfYear(DateTime(2015, 11, 31)) == 365);
  CHECK(dayOfYear(DateTime(2016, 11, 31)) == 366);
  CHECK(dayOfYear(DateTime(2015, 1, 1)) == 32);
}

TEST_CASE("verify approximateTransit near the International Date Line") {
  /**
   * For longitude ~177.24°E on Dec 1, 2025, solar transit falls just after UTC
   * midnight. The raw formula produces a tiny negative number that
   * normalizeToScale wraps to ~1. The fix should detect this and return a value
   * near 0, not near 1.
   */
  double longitude = 177.24;
  double jd = Astronomical::julianDay(2025, 12, 1);
  SolarCoordinates solar(jd);
  double m0 = Astronomical::approximateTransit(
      longitude, solar.apparentSiderealTime, solar.rightAscension);
  /**
   * Transit should be near 0 (just after UTC midnight), not near 1 (just before
   * UTC midnight). A small negative value (floating-point rounding) is
   * acceptable; what matters is that m0 was not incorrectly wrapped to ~1 by
   * normalizeToScale.
   */
  CHECK(m0 > -0.01);
  CHECK(m0 < 0.1);
}

TEST_CASE("verify Right Ascension Edge Case near the International Date Line") {
  // Coordinates near the International Date Line (Fiji region, longitude
  // ~177.24°E)
  Coordinates coordinates(42.74674252600066, 177.2401196144623);
  std::vector<SolarTime> solar;
  // Iterate over a full year starting Nov 1, 2025
  DateTime base(2025, 10, 1); // Nov 1, 2025
  for (int i = 0; i <= 365; i++) {
    solar.emplace_back(dateByAddingDays(base, i), coordinates);
  }

  for (size_t i = 1; i < solar.size(); i++) {
    const SolarTime &time = solar[i];
    const SolarTime &previousTime = solar[i - 1];
    CHECK(std::abs(time.transit - previousTime.transit) < 1.0 / 60);
    CHECK(std::abs(time.sunrise - previousTime.sunrise) < 2.0 / 60);
    CHECK(std::abs(time.sunset - previousTime.sunset) < 2.0 / 60);
  }
}

TEST_CASE("calculate the days since the winter or summer solstice") {
  CHECK(
      Astronomical::daysSinceSolstice(
          dayOfYear(DateTime(2016, 0, 1)), 2016, 1) == 11);
  CHECK(
      Astronomical::daysSinceSolstice(
          dayOfYear(DateTime(2015, 11, 31)), 2015, 1) == 10);
  CHECK(
      Astronomical::daysSinceSolstice(
          dayOfYear(DateTime(2016, 11, 31)), 2016, 1) == 10);
  CHECK(
      Astronomical::daysSinceSolstice(
          dayOfYear(DateTime(2016, 11, 21)), 2016, 1) == 0);
  CHECK(
      Astronomical::daysSinceSolstice(
          dayOfYear(DateTime(2016, 11, 22)), 2016, 1) == 1);
  CHECK(
      Astronomical::daysSinceSolstice(
          dayOfYear(DateTime(2016, 2, 1)), 2016, 1) == 71);
  CHECK(
      Astronomical::daysSinceSolstice(
          dayOfYear(DateTime(2015, 2, 1)), 2015, 1) == 70);
  CHECK(
      Astronomical::daysSinceSolstice(
          dayOfYear(DateTime(2016, 11, 20)), 2016, 1) == 365);
  CHECK(
      Astronomical::daysSinceSolstice(
          dayOfYear(DateTime(2015, 11, 20)), 2015, 1) == 364);

  CHECK(
      Astronomical::daysSinceSolstice(
          dayOfYear(DateTime(2015, 5, 21)), 2015, -1) == 0);
  CHECK(
      Astronomical::daysSinceSolstice(
          dayOfYear(DateTime(2016, 5, 21)), 2016, -1) == 0);
  CHECK(
      Astronomical::daysSinceSolstice(
          dayOfYear(DateTime(2015, 5, 20)), 2015, -1) == 364);
  CHECK(
      Astronomical::daysSinceSolstice(
          dayOfYear(DateTime(2016, 5, 20)), 2016, -1) == 365);
}