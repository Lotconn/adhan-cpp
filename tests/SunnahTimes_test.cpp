#include "doctest.h"

#include <UtcTime.hpp>
#include <adhan/CalculationMethod.hpp>
#include <adhan/Coordinates.hpp>
#include <adhan/HighLatitudeRule.hpp>
#include <adhan/PrayerTimes.hpp>
#include <adhan/SunnahTimes.hpp>

#include <chrono>

using namespace Adhan;
using namespace std::chrono;

TEST_CASE("getting sunnah times for the New York timezone") {
  Coordinates coords(35.775, -78.6336);
  CalculationParameters params = CalculationMethod::NorthAmerica();

  const year_month_day date1{2015y / July / 12d};
  PrayerTimes p1(coords, date1, params);
  /* 2015-07-12 20:32 America/New_York  =  2015-07-13 00:32Z */
  CHECK(isUtc(p1.maghrib, 2015y / July / 13d, 0h + 32min));

  const year_month_day date2{2015y / July / 13d};
  PrayerTimes p2(coords, date2, params);
  /* 2015-07-13 04:43 America/New_York  =  2015-07-13 08:43Z */
  CHECK(isUtc(p2.fajr, 2015y / July / 13d, 8h + 43min));

  SunnahTimes sunnah(p1);
  /* 2015-07-13 00:38 America/New_York  =  2015-07-13 04:38Z */
  CHECK(isUtc(sunnah.middleOfTheNight, 2015y / July / 13d, 4h + 38min));
  /* 2015-07-13 01:59 America/New_York  =  2015-07-13 05:59Z */
  CHECK(isUtc(sunnah.lastThirdOfTheNight, 2015y / July / 13d, 5h + 59min));
}

TEST_CASE("getting sunnah times for the London timezone") {
  Coordinates coords(51.5074, -0.1278);
  CalculationParameters params = CalculationMethod::MoonsightingCommittee();

  const year_month_day date1{2016y / December / 31d};
  PrayerTimes p1(coords, date1, params);
  /* 2016-12-31 16:04 Europe/London  =  2016-12-31 16:04Z */
  CHECK(isUtc(p1.maghrib, 2016y / December / 31d, 16h + 4min));

  const year_month_day date2{2017y / January / 1d};
  PrayerTimes p2(coords, date2, params);
  /* 2017-01-01 06:25 Europe/London  =  2017-01-01 06:25Z */
  CHECK(isUtc(p2.fajr, 2017y / January / 1d, 6h + 25min));

  SunnahTimes sunnah(p1);
  /* 2016-12-31 23:15 Europe/London  =  2016-12-31 23:15Z */
  CHECK(isUtc(sunnah.middleOfTheNight, 2016y / December / 31d, 23h + 15min));
  /* 2017-01-01 01:38 Europe/London  =  2017-01-01 01:38Z */
  CHECK(isUtc(sunnah.lastThirdOfTheNight, 2017y / January / 1d, 1h + 38min));
}

TEST_CASE("getting sunnah times for the Oslo timezone") {
  Coordinates coords(59.9094, 10.7349);
  CalculationParameters params = CalculationMethod::MuslimWorldLeague();
  params.highLatitudeRule = HighLatitudeRule::MiddleOfTheNight;

  const year_month_day date1{2016y / July / 1d};
  PrayerTimes p1(coords, date1, params);
  /* 2016-07-01 22:41 Europe/Oslo  =  2016-07-01 20:41Z */
  CHECK(isUtc(p1.maghrib, 2016y / July / 1d, 20h + 41min));

  const year_month_day date2{2016y / July / 2d};
  PrayerTimes p2(coords, date2, params);
  /* 2016-07-02 01:20 Europe/Oslo  =  2016-07-01 23:20Z */
  CHECK(isUtc(p2.fajr, 2016y / July / 1d, 23h + 20min));

  SunnahTimes sunnah(p1);
  /* 2016-07-02 00:01 Europe/Oslo  =  2016-07-01 22:01Z */
  CHECK(isUtc(sunnah.middleOfTheNight, 2016y / July / 1d, 22h + 1min));
  /* 2016-07-02 00:27 Europe/Oslo  =  2016-07-01 22:27Z */
  CHECK(isUtc(sunnah.lastThirdOfTheNight, 2016y / July / 1d, 22h + 27min));
}

/**
 * This pair used to sit on a daylight saving boundary on purpose. Back then
 * dateByAddingDays rebuilt a date from its local wall clock fields, so a
 * transition could move the result by an hour. That path is gone, the
 * library reads no time zone at all now, so what is left is simply two more
 * locations worth of expected values.
 */
TEST_CASE("getting sunnah times for the San Francisco timezone") {
  Coordinates coords(37.7749, -122.4194);
  CalculationParameters params = CalculationMethod::NorthAmerica();

  const year_month_day date1{2017y / March / 11d};
  PrayerTimes p1(coords, date1, params);
  /* 2017-03-11 05:14 America/Los_Angeles  =  2017-03-11 13:14Z */
  CHECK(isUtc(p1.fajr, 2017y / March / 11d, 13h + 14min));
  /* 2017-03-11 18:13 America/Los_Angeles  =  2017-03-12 02:13Z */
  CHECK(isUtc(p1.maghrib, 2017y / March / 12d, 2h + 13min));

  const year_month_day date2{2017y / March / 12d};
  PrayerTimes p2(coords, date2, params);
  /* 2017-03-12 06:13 America/Los_Angeles  =  2017-03-12 13:13Z */
  CHECK(isUtc(p2.fajr, 2017y / March / 12d, 13h + 13min));
  /* 2017-03-12 19:14 America/Los_Angeles  =  2017-03-13 02:14Z */
  CHECK(isUtc(p2.maghrib, 2017y / March / 13d, 2h + 14min));

  SunnahTimes sunnah(p1);
  /* 2017-03-11 23:43 America/Los_Angeles  =  2017-03-12 07:43Z */
  CHECK(isUtc(sunnah.middleOfTheNight, 2017y / March / 12d, 7h + 43min));
  /* 2017-03-12 01:33 America/Los_Angeles  =  2017-03-12 09:33Z */
  CHECK(isUtc(sunnah.lastThirdOfTheNight, 2017y / March / 12d, 9h + 33min));
}

TEST_CASE("getting sunnah times for the Paris timezone") {
  Coordinates coords(48.8566, 2.3522);
  CalculationParameters params = CalculationMethod::MuslimWorldLeague();
  params.highLatitudeRule = HighLatitudeRule::SeventhOfTheNight;

  const year_month_day date1{2015y / October / 24d};
  PrayerTimes p1(coords, date1, params);
  /* 2015-10-24 06:38 Europe/Paris  =  2015-10-24 04:38Z */
  CHECK(isUtc(p1.fajr, 2015y / October / 24d, 4h + 38min));
  /* 2015-10-24 18:45 Europe/Paris  =  2015-10-24 16:45Z */
  CHECK(isUtc(p1.maghrib, 2015y / October / 24d, 16h + 45min));

  const year_month_day date2{2015y / October / 25d};
  PrayerTimes p2(coords, date2, params);
  /* 2015-10-25 05:40 Europe/Paris  =  2015-10-25 04:40Z */
  CHECK(isUtc(p2.fajr, 2015y / October / 25d, 4h + 40min));
  /* 2015-10-25 17:43 Europe/Paris  =  2015-10-25 16:43Z */
  CHECK(isUtc(p2.maghrib, 2015y / October / 25d, 16h + 43min));

  SunnahTimes sunnah(p1);
  /* 2015-10-25 00:43 Europe/Paris  =  2015-10-24 22:43Z */
  CHECK(isUtc(sunnah.middleOfTheNight, 2015y / October / 24d, 22h + 43min));
  /* 2015-10-25 02:42 Europe/Paris  =  2015-10-25 00:42Z */
  CHECK(isUtc(sunnah.lastThirdOfTheNight, 2015y / October / 25d, 0h + 42min));
}
