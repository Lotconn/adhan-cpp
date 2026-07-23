#include "doctest.h"

#include <CalculationMethod.hpp>
#include <Coordinates.hpp>
#include <HighLatitudeRule.hpp>
#include <JSDate.hpp>
#include <MomentFormat.hpp>
#include <PrayerTimes.hpp>
#include <SunnahTimes.hpp>

TEST_CASE("getting sunnah times for the New York timezone") {
  Coordinates coords(35.775, -78.6336);
  CalculationParameters params = CalculationMethod::NorthAmerica();

  JSDate date1(2015, 6, 12);
  PrayerTimes p1(coords, date1, params);
  CHECK(formatInZone(p1.maghrib, "America/New_York", "M/D/YY, h:mm A") ==
        "7/12/15, 8:32 PM");

  JSDate date2(2015, 6, 13);
  PrayerTimes p2(coords, date2, params);
  CHECK(formatInZone(p2.fajr, "America/New_York", "M/D/YY, h:mm A") ==
        "7/13/15, 4:43 AM");

  SunnahTimes sunnah(p1);
  CHECK(formatInZone(sunnah.middleOfTheNight, "America/New_York",
                     "M/D/YY, h:mm A") == "7/13/15, 12:38 AM");
  CHECK(formatInZone(sunnah.lastThirdOfTheNight, "America/New_York",
                     "M/D/YY, h:mm A") == "7/13/15, 1:59 AM");
}

TEST_CASE("getting sunnah times for the London timezone") {
  Coordinates coords(51.5074, -0.1278);
  CalculationParameters params = CalculationMethod::MoonsightingCommittee();

  JSDate date1(2016, 11, 31);
  PrayerTimes p1(coords, date1, params);
  CHECK(formatInZone(p1.maghrib, "Europe/London", "M/D/YY, h:mm A") ==
        "12/31/16, 4:04 PM");

  JSDate date2(2017, 0, 1);
  PrayerTimes p2(coords, date2, params);
  CHECK(formatInZone(p2.fajr, "Europe/London", "M/D/YY, h:mm A") ==
        "1/1/17, 6:25 AM");

  SunnahTimes sunnah(p1);
  CHECK(formatInZone(sunnah.middleOfTheNight, "Europe/London",
                     "M/D/YY, h:mm A") == "12/31/16, 11:15 PM");
  CHECK(formatInZone(sunnah.lastThirdOfTheNight, "Europe/London",
                     "M/D/YY, h:mm A") == "1/1/17, 1:38 AM");
}

TEST_CASE("getting sunnah times for the Oslo timezone") {
  Coordinates coords(59.9094, 10.7349);
  CalculationParameters params = CalculationMethod::MuslimWorldLeague();
  params.highLatitudeRule = HighLatitudeRule::MiddleOfTheNight;

  JSDate date1(2016, 6, 1);
  PrayerTimes p1(coords, date1, params);
  CHECK(formatInZone(p1.maghrib, "Europe/Oslo", "M/D/YY, h:mm A") ==
        "7/1/16, 10:41 PM");

  JSDate date2(2016, 6, 2);
  PrayerTimes p2(coords, date2, params);
  CHECK(formatInZone(p2.fajr, "Europe/Oslo", "M/D/YY, h:mm A") ==
        "7/2/16, 1:20 AM");

  SunnahTimes sunnah(p1);
  CHECK(formatInZone(sunnah.middleOfTheNight, "Europe/Oslo",
                     "M/D/YY, h:mm A") == "7/2/16, 12:01 AM");
  CHECK(formatInZone(sunnah.lastThirdOfTheNight, "Europe/Oslo",
                     "M/D/YY, h:mm A") == "7/2/16, 12:27 AM");
}

TEST_CASE("getting sunnah times for US DST change") {
  Coordinates coords(37.7749, -122.4194);
  CalculationParameters params = CalculationMethod::NorthAmerica();

  JSDate date1(2017, 2, 11);
  PrayerTimes p1(coords, date1, params);
  CHECK(formatInZone(p1.fajr, "America/Los_Angeles", "M/D/YY, h:mm A") ==
        "3/11/17, 5:14 AM");
  CHECK(formatInZone(p1.maghrib, "America/Los_Angeles", "M/D/YY, h:mm A") ==
        "3/11/17, 6:13 PM");

  JSDate date2(2017, 2, 12);
  PrayerTimes p2(coords, date2, params);
  CHECK(formatInZone(p2.fajr, "America/Los_Angeles", "M/D/YY, h:mm A") ==
        "3/12/17, 6:13 AM");
  CHECK(formatInZone(p2.maghrib, "America/Los_Angeles", "M/D/YY, h:mm A") ==
        "3/12/17, 7:14 PM");

  SunnahTimes sunnah(p1);
  CHECK(formatInZone(sunnah.middleOfTheNight, "America/Los_Angeles",
                     "M/D/YY, h:mm A") == "3/11/17, 11:43 PM");
  CHECK(formatInZone(sunnah.lastThirdOfTheNight, "America/Los_Angeles",
                     "M/D/YY, h:mm A") == "3/12/17, 1:33 AM");
}

TEST_CASE("getting sunnah times for Europe DST change") {
  Coordinates coords(48.8566, 2.3522);
  CalculationParameters params = CalculationMethod::MuslimWorldLeague();
  params.highLatitudeRule = HighLatitudeRule::SeventhOfTheNight;

  JSDate date1(2015, 9, 24);
  PrayerTimes p1(coords, date1, params);
  CHECK(formatInZone(p1.fajr, "Europe/Paris", "M/D/YY, h:mm A") ==
        "10/24/15, 6:38 AM");
  CHECK(formatInZone(p1.maghrib, "Europe/Paris", "M/D/YY, h:mm A") ==
        "10/24/15, 6:45 PM");

  JSDate date2(2015, 9, 25);
  PrayerTimes p2(coords, date2, params);
  CHECK(formatInZone(p2.fajr, "Europe/Paris", "M/D/YY, h:mm A") ==
        "10/25/15, 5:40 AM");
  CHECK(formatInZone(p2.maghrib, "Europe/Paris", "M/D/YY, h:mm A") ==
        "10/25/15, 5:43 PM");

  SunnahTimes sunnah(p1);
  CHECK(formatInZone(sunnah.middleOfTheNight, "Europe/Paris",
                     "M/D/YY, h:mm A") == "10/25/15, 12:43 AM");
  CHECK(formatInZone(sunnah.lastThirdOfTheNight, "Europe/Paris",
                     "M/D/YY, h:mm A") == "10/25/15, 2:42 AM");
}