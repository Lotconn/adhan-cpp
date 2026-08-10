#include "doctest.h"
#include <UtcTime.hpp>

#include <adhan/CalculationMethod.hpp>
#include <adhan/Coordinates.hpp>
#include <adhan/DateUtils.hpp>
#include <adhan/PolarCircleResolution.hpp>
#include <adhan/PrayerTimes.hpp>

#include <array>
#include <chrono>

using namespace Adhan;
using namespace std::chrono;

namespace {
const std::array<OptInstant PrayerTimes::*, 4> kPrayersToCheck = {
    &PrayerTimes::fajr, &PrayerTimes::sunrise, &PrayerTimes::maghrib,
    &PrayerTimes::isha};
} // namespace

struct PolarCircleFixture {
  /**
   * These used to carry a 20:00:00 wall clock time. Nothing ever read it,
   * the library only wanted the calendar day, so it is gone.
   */
  year_month_day regularDate{2020y / May / 15d};
  year_month_day dateAffectedByPolarNight{2020y / December / 21d};
  year_month_day dateAffectedByMidnightSun{2020y / June / 21d};
  Coordinates regularCoordinates{31.947351, 35.227163};
  Coordinates ArjeplogSweden{66.7222444, 17.7189};
  Coordinates AmundsenScottAntarctic{-84.996, 0.01013};
  CalculationParameters unresolvedParams =
      CalculationMethod::MuslimWorldLeague();
  CalculationParameters aqrabBaladParams =
      CalculationMethod::MuslimWorldLeague();
  CalculationParameters aqrabYaumParams =
      CalculationMethod::MuslimWorldLeague();

  PolarCircleFixture() {
    aqrabBaladParams.polarCircleResolution = PolarCircleResolution::AqrabBalad;
    aqrabYaumParams.polarCircleResolution = PolarCircleResolution::AqrabYaum;
  }
};

// --- Regular computation ---

/**
 * The TS suite spies on `polarCircleResolvedValues` to assert it never runs
 * on an ordinary day. There is no cheap equivalent of that in C++ short of
 * putting a counter in shipped code, so these check the consequence instead:
 * picking a resolver must not move any prayer time on a day that did not
 * need resolving.
 *
 * That is not a weaker test. Neither resolver returns the day it was handed.
 * AqrabYaum starts its search a day out, and AqrabBalad steps half a degree
 * toward the equator before its first try, so a resolver that ran when it
 * should not have would show up here as a shifted time.
 */
void checkSameTimes(const PrayerTimes &actual, const PrayerTimes &expected) {
  CHECK(actual.fajr == expected.fajr);
  CHECK(actual.sunrise == expected.sunrise);
  CHECK(actual.dhuhr == expected.dhuhr);
  CHECK(actual.asr == expected.asr);
  CHECK(actual.sunset == expected.sunset);
  CHECK(actual.maghrib == expected.maghrib);
  CHECK(actual.isha == expected.isha);
}

TEST_CASE_FIXTURE(
    PolarCircleFixture,
    "Regular computation: a resolver leaves a date alone when that date is "
    "affected neither by the polar night nor by the midnight sun") {
  const PrayerTimes reference(ArjeplogSweden, regularDate, unresolvedParams);

  checkSameTimes(
      PrayerTimes(ArjeplogSweden, regularDate, aqrabBaladParams), reference);
  checkSameTimes(
      PrayerTimes(ArjeplogSweden, regularDate, aqrabYaumParams), reference);
}

TEST_CASE_FIXTURE(
    PolarCircleFixture,
    "Regular computation: a resolver leaves a location alone when that "
    "location is outside the polar circles") {
  const PrayerTimes reference(
      regularCoordinates, dateAffectedByPolarNight, unresolvedParams);

  checkSameTimes(
      PrayerTimes(
          regularCoordinates, dateAffectedByPolarNight, aqrabBaladParams),
      reference);
  checkSameTimes(
      PrayerTimes(
          regularCoordinates, dateAffectedByPolarNight, aqrabYaumParams),
      reference);
}

// --- Midnight Sun case ---

TEST_CASE_FIXTURE(
    PolarCircleFixture,
    "Midnight Sun case: should fail to compute targeted prayer "
    "times with the unresolved resolver") {
  PrayerTimes prayersTimes(
      ArjeplogSweden, dateAffectedByMidnightSun, unresolvedParams);

  for (auto member : kPrayersToCheck) {
    CHECK(isValidDate(prayersTimes.*member) == false);
  }
}

TEST_CASE_FIXTURE(
    PolarCircleFixture,
    "Midnight Sun case: should succeed in computing all prayer "
    "times with the aqrabBalad resolver") {
  PrayerTimes prayersTimes(
      ArjeplogSweden, dateAffectedByMidnightSun, aqrabBaladParams);

  for (auto member : kPrayersToCheck) {
    CHECK(isValidDate(prayersTimes.*member) == true);
  }
}

TEST_CASE_FIXTURE(
    PolarCircleFixture,
    "Midnight Sun case: should succeed in computing all prayer "
    "times with the aqrabYaum resolver") {
  PrayerTimes prayersTimes(
      ArjeplogSweden, dateAffectedByMidnightSun, aqrabYaumParams);

  for (auto member : kPrayersToCheck) {
    CHECK(isValidDate(prayersTimes.*member) == true);
  }
}

// --- Polar Night case ---

TEST_CASE_FIXTURE(
    PolarCircleFixture,
    "Polar Night case: should fail to compute targeted prayer "
    "times with the unresolved resolver") {
  PrayerTimes prayersTimes(
      AmundsenScottAntarctic, dateAffectedByPolarNight, unresolvedParams);

  for (auto member : kPrayersToCheck) {
    CHECK(isValidDate(prayersTimes.*member) == false);
  }
}

TEST_CASE_FIXTURE(
    PolarCircleFixture,
    "Polar Night case: should succeed in computing all prayer "
    "times with the aqrabBalad resolver") {
  PrayerTimes prayersTimes(
      AmundsenScottAntarctic, dateAffectedByPolarNight, aqrabBaladParams);

  for (auto member : kPrayersToCheck) {
    CHECK(isValidDate(prayersTimes.*member) == true);
  }
}

TEST_CASE_FIXTURE(
    PolarCircleFixture,
    "Polar Night case: should succeed in computing all prayer "
    "times with the aqrabYaum resolver") {
  PrayerTimes prayersTimes(
      AmundsenScottAntarctic, dateAffectedByPolarNight, aqrabYaumParams);

  for (auto member : kPrayersToCheck) {
    CHECK(isValidDate(prayersTimes.*member) == true);
  }
}

TEST_CASE("Polar Night case: calculating times for the polar circle") {
  Coordinates coordinates(66.7222444, 17.7189);
  CalculationParameters params = CalculationMethod::MuslimWorldLeague();
  params.polarCircleResolution = PolarCircleResolution::AqrabYaum;
  params.highLatitudeRule = HighLatitudeRule::SeventhOfTheNight;
  const year_month_day date{2020y / June / 21d};

  PrayerTimes p(coordinates, date, params);
  /* 2020-06-21 00:40 Europe/Stockholm  =  2020-06-20 22:40Z */
  CHECK(isUtc(p.fajr, 2020y / June / 20d, 22h + 40min));
  /* 2020-06-21 00:54 Europe/Stockholm  =  2020-06-20 22:54Z */
  CHECK(isUtc(p.sunrise, 2020y / June / 20d, 22h + 54min));
  /* 2020-06-21 12:55 Europe/Stockholm  =  2020-06-21 10:55Z */
  CHECK(isUtc(p.dhuhr, 2020y / June / 21d, 10h + 55min));
  /* 2020-06-21 17:49 Europe/Stockholm  =  2020-06-21 15:49Z */
  CHECK(isUtc(p.asr, 2020y / June / 21d, 15h + 49min));
  /* 2020-06-21 23:36 Europe/Stockholm  =  2020-06-21 21:36Z */
  CHECK(isUtc(p.maghrib, 2020y / June / 21d, 21h + 36min));
  /* 2020-06-21 23:51 Europe/Stockholm  =  2020-06-21 21:51Z */
  CHECK(isUtc(p.isha, 2020y / June / 21d, 21h + 51min));
}
