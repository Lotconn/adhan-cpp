#include "MomentFormat.hpp"
#include "doctest.h"

#include <adhan/CalculationMethod.hpp>
#include <adhan/Coordinates.hpp>
#include <adhan/DateTime.hpp>
#include <adhan/DateUtils.hpp>
#include <adhan/PolarCircleResolution.hpp>
#include <adhan/PrayerTimes.hpp>

#include <array>

using namespace Adhan;

namespace {
const std::array<DateTime PrayerTimes::*, 4> kPrayersToCheck = {
    &PrayerTimes::fajr, &PrayerTimes::sunrise, &PrayerTimes::maghrib,
    &PrayerTimes::isha};
} // namespace

struct PolarCircleFixture {
  DateTime regularDate{2020, 4, 15, 20, 0, 0};
  DateTime dateAffectedByPolarNight{2020, 11, 21, 20, 0, 0};
  DateTime dateAffectedByMidnightSun{2020, 5, 21, 20, 0, 0};
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

TEST_CASE_FIXTURE(
    PolarCircleFixture,
    "Regular computation: should not attempt any resolution if "
    "the resolver is set to unresolved") {

  /* So that the compiler does not complain, */
  // NOLINTBEGIN
  int before = polarCircleResolvedValuesCallCount;
  // NOLINTEND

  PrayerTimes prayersTimes1(
      ArjeplogSweden, dateAffectedByMidnightSun, unresolvedParams);
  PrayerTimes prayersTimes2(
      ArjeplogSweden, dateAffectedByMidnightSun, unresolvedParams);

  CHECK(polarCircleResolvedValuesCallCount == before);
}

TEST_CASE_FIXTURE(
    PolarCircleFixture,
    "Regular computation: should not attempt any resolution if the date is "
    "affected neither by the polar night nor by the midnight sun") {
  int before = polarCircleResolvedValuesCallCount;

  PrayerTimes prayersTimes1(ArjeplogSweden, regularDate, aqrabBaladParams);
  PrayerTimes prayersTimes2(ArjeplogSweden, regularDate, aqrabYaumParams);

  CHECK(polarCircleResolvedValuesCallCount == before);
}

TEST_CASE_FIXTURE(
    PolarCircleFixture,
    "Regular computation: should not make any search if the "
    "location is outside the polar circles") {
  int before = polarCircleResolvedValuesCallCount;

  PrayerTimes prayersTimes1(
      regularCoordinates, dateAffectedByPolarNight, aqrabBaladParams);
  PrayerTimes prayersTimes2(
      regularCoordinates, dateAffectedByPolarNight, aqrabYaumParams);

  CHECK(polarCircleResolvedValuesCallCount == before);
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

#ifndef ADHAN_USE_CTIME_FALLBACK

TEST_CASE("Polar Night case: calculating times for the polar circle") {
  Coordinates coordinates(66.7222444, 17.7189);
  CalculationParameters params = CalculationMethod::MuslimWorldLeague();
  params.polarCircleResolution = PolarCircleResolution::AqrabYaum;
  params.highLatitudeRule = HighLatitudeRule::SeventhOfTheNight;
  DateTime date(2020, 5, 21);

  PrayerTimes p(coordinates, date, params);
  CHECK(
      formatInZone(p.fajr, "Europe/Stockholm", "MMMM DD, YYYY h:mm A") ==
      "June 21, 2020 12:40 AM");
  CHECK(
      formatInZone(p.sunrise, "Europe/Stockholm", "MMMM DD, YYYY h:mm A") ==
      "June 21, 2020 12:54 AM");
  CHECK(
      formatInZone(p.dhuhr, "Europe/Stockholm", "MMMM DD, YYYY h:mm A") ==
      "June 21, 2020 12:55 PM");
  CHECK(
      formatInZone(p.asr, "Europe/Stockholm", "MMMM DD, YYYY h:mm A") ==
      "June 21, 2020 5:49 PM");
  CHECK(
      formatInZone(p.maghrib, "Europe/Stockholm", "MMMM DD, YYYY h:mm A") ==
      "June 21, 2020 11:36 PM");
  CHECK(
      formatInZone(p.isha, "Europe/Stockholm", "MMMM DD, YYYY h:mm A") ==
      "June 21, 2020 11:51 PM");
}

#endif

TEST_CASE("calculating prayer times near the International Date Line") {
  /**
   * Coordinates near the International Date Line (longitude ~177.24°E).
   * Prior to the approximateTransit fix, the solar transit would be
   * miscalculated as just before UTC midnight instead of just after, causing
   * prayer times to be off by a full day. Verify that all prayer times are in
   * the correct order.
   */
  CalculationParameters params = CalculationMethod::MuslimWorldLeague();
  params.madhab = Madhab::Shafi;
  params.highLatitudeRule = HighLatitudeRule::TwilightAngle;

  DateTime date(2025, 11, 1); // Dec 1, 2025

  PrayerTimes p1(
      Coordinates(42.74674252600066, 177.2401196144623), date, params);
  CHECK(p1.fajr.getTime() < p1.sunrise.getTime());
  CHECK(p1.sunrise.getTime() < p1.dhuhr.getTime());
  CHECK(p1.dhuhr.getTime() < p1.asr.getTime());
  CHECK(p1.asr.getTime() < p1.maghrib.getTime());
  CHECK(p1.maghrib.getTime() < p1.isha.getTime());

  PrayerTimes p2(
      Coordinates(47.082209457885355, 177.24642294208638), date, params);
  CHECK(p2.fajr.getTime() < p2.sunrise.getTime());
  CHECK(p2.sunrise.getTime() < p2.dhuhr.getTime());
  CHECK(p2.dhuhr.getTime() < p2.asr.getTime());
  CHECK(p2.asr.getTime() < p2.maghrib.getTime());
  CHECK(p2.maghrib.getTime() < p2.isha.getTime());
}