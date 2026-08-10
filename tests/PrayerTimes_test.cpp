#include "DateUtils.hpp"
#include "HighLatitudeRule.hpp"
#include "doctest.h"

#include <UtcTime.hpp>
#include <adhan/CalculationMethod.hpp>
#include <adhan/Coordinates.hpp>
#include <adhan/Madhab.hpp>
#include <adhan/PrayerTimes.hpp>

#include <chrono>

using namespace Adhan;
using namespace std::chrono;
TEST_CASE("calculating prayer times") {
  const year_month_day date{2015y/July/12d};
  CalculationParameters params = CalculationMethod::NorthAmerica();
  params.madhab = Madhab::Hanafi;
  PrayerTimes p(Coordinates(35.775, -78.6336), date, params);

  /* 2015-07-12 04:42 America/New_York  =  2015-07-12 08:42Z */
  CHECK(isUtc(p.fajr, 2015y/July/12d, 8h + 42min));
  /* 2015-07-12 06:08 America/New_York  =  2015-07-12 10:08Z */
  CHECK(isUtc(p.sunrise, 2015y/July/12d, 10h + 8min));
  /* 2015-07-12 13:21 America/New_York  =  2015-07-12 17:21Z */
  CHECK(isUtc(p.dhuhr, 2015y/July/12d, 17h + 21min));
  /* 2015-07-12 18:22 America/New_York  =  2015-07-12 22:22Z */
  CHECK(isUtc(p.asr, 2015y/July/12d, 22h + 22min));
  /* 2015-07-12 20:32 America/New_York  =  2015-07-13 00:32Z */
  CHECK(isUtc(p.maghrib, 2015y/July/13d, 0h + 32min));
  /* 2015-07-12 21:57 America/New_York  =  2015-07-13 01:57Z */
  CHECK(isUtc(p.isha, 2015y/July/13d, 1h + 57min));
  /* 2015-07-12 21:57 America/New_York  =  2015-07-13 01:57Z */
  CHECK(isUtc(p.isha, 2015y/July/13d, 1h + 57min));
}

TEST_CASE("using offsets to manually adjust prayer times") {
  const year_month_day date{2015y/December/1d};
  CalculationParameters params = CalculationMethod::MuslimWorldLeague();
  params.madhab = Madhab::Shafi;
  PrayerTimes p(Coordinates(35.775, -78.6336), date, params);

  /* 2015-12-01 05:35 America/New_York  =  2015-12-01 10:35Z */
  CHECK(isUtc(p.fajr, 2015y/December/1d, 10h + 35min));
  /* 2015-12-01 07:06 America/New_York  =  2015-12-01 12:06Z */
  CHECK(isUtc(p.sunrise, 2015y/December/1d, 12h + 6min));
  /* 2015-12-01 12:05 America/New_York  =  2015-12-01 17:05Z */
  CHECK(isUtc(p.dhuhr, 2015y/December/1d, 17h + 5min));
  /* 2015-12-01 14:42 America/New_York  =  2015-12-01 19:42Z */
  CHECK(isUtc(p.asr, 2015y/December/1d, 19h + 42min));
  /* 2015-12-01 17:01 America/New_York  =  2015-12-01 22:01Z */
  CHECK(isUtc(p.maghrib, 2015y/December/1d, 22h + 1min));
  /* 2015-12-01 18:26 America/New_York  =  2015-12-01 23:26Z */
  CHECK(isUtc(p.isha, 2015y/December/1d, 23h + 26min));

  params.adjustments.fajr = 10;
  params.adjustments.sunrise = 10;
  params.adjustments.dhuhr = 10;
  params.adjustments.asr = 10;
  params.adjustments.maghrib = 10;
  params.adjustments.isha = 10;

  PrayerTimes p2(Coordinates(35.775, -78.6336), date, params);
  /* 2015-12-01 05:45 America/New_York  =  2015-12-01 10:45Z */
  CHECK(isUtc(p2.fajr, 2015y/December/1d, 10h + 45min));
  /* 2015-12-01 07:16 America/New_York  =  2015-12-01 12:16Z */
  CHECK(isUtc(p2.sunrise, 2015y/December/1d, 12h + 16min));
  /* 2015-12-01 12:15 America/New_York  =  2015-12-01 17:15Z */
  CHECK(isUtc(p2.dhuhr, 2015y/December/1d, 17h + 15min));
  /* 2015-12-01 14:52 America/New_York  =  2015-12-01 19:52Z */
  CHECK(isUtc(p2.asr, 2015y/December/1d, 19h + 52min));
  /* 2015-12-01 17:11 America/New_York  =  2015-12-01 22:11Z */
  CHECK(isUtc(p2.maghrib, 2015y/December/1d, 22h + 11min));
  /* 2015-12-01 18:36 America/New_York  =  2015-12-01 23:36Z */
  CHECK(isUtc(p2.isha, 2015y/December/1d, 23h + 36min));
}

TEST_CASE(
    "calculating prayer times using the Moonsighting Committee "
    "calculation method") {
  // Values from http://www.moonsighting.com/pray.php
  const year_month_day date{2016y/January/31d};
  PrayerTimes p(
      Coordinates(35.775, -78.6336), date,
      CalculationMethod::MoonsightingCommittee());

  /* 2016-01-31 05:48 America/New_York  =  2016-01-31 10:48Z */
  CHECK(isUtc(p.fajr, 2016y/January/31d, 10h + 48min));
  /* 2016-01-31 07:16 America/New_York  =  2016-01-31 12:16Z */
  CHECK(isUtc(p.sunrise, 2016y/January/31d, 12h + 16min));
  /* 2016-01-31 12:33 America/New_York  =  2016-01-31 17:33Z */
  CHECK(isUtc(p.dhuhr, 2016y/January/31d, 17h + 33min));
  /* 2016-01-31 15:20 America/New_York  =  2016-01-31 20:20Z */
  CHECK(isUtc(p.asr, 2016y/January/31d, 20h + 20min));
  /* 2016-01-31 17:43 America/New_York  =  2016-01-31 22:43Z */
  CHECK(isUtc(p.maghrib, 2016y/January/31d, 22h + 43min));
  /* 2016-01-31 19:05 America/New_York  =  2016-02-01 00:05Z */
  CHECK(isUtc(p.isha, 2016y/February/1d, 0h + 5min));
}

TEST_CASE(
    "calculating Moonsighting Committee prayer times at a high latitude "
    "location") {
  // Values from http://www.moonsighting.com/pray.php
  const year_month_day date{2016y/January/1d};
  CalculationParameters params = CalculationMethod::MoonsightingCommittee();
  params.madhab = Madhab::Hanafi;
  PrayerTimes p(Coordinates(59.9094, 10.7349), date, params);

  /* 2016-01-01 07:34 Europe/Oslo  =  2016-01-01 06:34Z */
  CHECK(isUtc(p.fajr, 2016y/January/1d, 6h + 34min));
  /* 2016-01-01 09:19 Europe/Oslo  =  2016-01-01 08:19Z */
  CHECK(isUtc(p.sunrise, 2016y/January/1d, 8h + 19min));
  /* 2016-01-01 12:25 Europe/Oslo  =  2016-01-01 11:25Z */
  CHECK(isUtc(p.dhuhr, 2016y/January/1d, 11h + 25min));
  /* 2016-01-01 13:36 Europe/Oslo  =  2016-01-01 12:36Z */
  CHECK(isUtc(p.asr, 2016y/January/1d, 12h + 36min));
  /* 2016-01-01 15:25 Europe/Oslo  =  2016-01-01 14:25Z */
  CHECK(isUtc(p.maghrib, 2016y/January/1d, 14h + 25min));
  /* 2016-01-01 17:02 Europe/Oslo  =  2016-01-01 16:02Z */
  CHECK(isUtc(p.isha, 2016y/January/1d, 16h + 2min));
}

TEST_CASE("calculating times for turkey method") {
  // values from
  // https://namazvakitleri.diyanet.gov.tr/en-US/9541/prayer-time-for-istanbul
  const year_month_day date{2020y/April/16d};
  CalculationParameters params = CalculationMethod::Turkey();
  PrayerTimes p(Coordinates(41.005616, 28.97638), date, params);

  /* 2020-04-16 04:44 Europe/Istanbul  =  2020-04-16 01:44Z */
  CHECK(isUtc(p.fajr, 2020y/April/16d, 1h + 44min));
  /* 2020-04-16 06:16 Europe/Istanbul  =  2020-04-16 03:16Z */
  CHECK(isUtc(p.sunrise, 2020y/April/16d, 3h + 16min));
  /* 2020-04-16 13:09 Europe/Istanbul  =  2020-04-16 10:09Z */
  CHECK(isUtc(p.dhuhr, 2020y/April/16d, 10h + 9min));
  /**
   * 2020-04-16 16:53 Europe/Istanbul  =  2020-04-16 13:53Z
   * Original time 4:52 PM
   */
  CHECK(isUtc(p.asr, 2020y/April/16d, 13h + 53min));
  /* 2020-04-16 19:52 Europe/Istanbul  =  2020-04-16 16:52Z */
  CHECK(isUtc(p.maghrib, 2020y/April/16d, 16h + 52min));
  /**
   * 2020-04-16 21:19 Europe/Istanbul  =  2020-04-16 18:19Z
   * Original time 9:18 PM
   */
  CHECK(isUtc(p.isha, 2020y/April/16d, 18h + 19min));
}

TEST_CASE("calculating times for the egyptian method") {
  const year_month_day date{2020y/January/1d};
  CalculationParameters params = CalculationMethod::Egyptian();
  PrayerTimes p(Coordinates(30.028703, 31.249528), date, params);

  /* 2020-01-01 05:18 Africa/Cairo  =  2020-01-01 03:18Z */
  CHECK(isUtc(p.fajr, 2020y/January/1d, 3h + 18min));
  /* 2020-01-01 06:51 Africa/Cairo  =  2020-01-01 04:51Z */
  CHECK(isUtc(p.sunrise, 2020y/January/1d, 4h + 51min));
  /* 2020-01-01 11:59 Africa/Cairo  =  2020-01-01 09:59Z */
  CHECK(isUtc(p.dhuhr, 2020y/January/1d, 9h + 59min));
  /* 2020-01-01 14:47 Africa/Cairo  =  2020-01-01 12:47Z */
  CHECK(isUtc(p.asr, 2020y/January/1d, 12h + 47min));
  /* 2020-01-01 17:06 Africa/Cairo  =  2020-01-01 15:06Z */
  CHECK(isUtc(p.maghrib, 2020y/January/1d, 15h + 6min));
  /* 2020-01-01 18:29 Africa/Cairo  =  2020-01-01 16:29Z */
  CHECK(isUtc(p.isha, 2020y/January/1d, 16h + 29min));
}

TEST_CASE("calculating times for the singapore method") {
  const year_month_day date{2021y/June/14d};
  CalculationParameters params = CalculationMethod::Singapore();
  PrayerTimes p(Coordinates(3.7333333333, 101.3833333333), date, params);

  /* 2021-06-14 05:41 Asia/Kuala_Lumpur  =  2021-06-13 21:41Z */
  CHECK(isUtc(p.fajr, 2021y/June/13d, 21h + 41min));
  /* 2021-06-14 07:05 Asia/Kuala_Lumpur  =  2021-06-13 23:05Z */
  CHECK(isUtc(p.sunrise, 2021y/June/13d, 23h + 5min));
  /* 2021-06-14 13:16 Asia/Kuala_Lumpur  =  2021-06-14 05:16Z */
  CHECK(isUtc(p.dhuhr, 2021y/June/14d, 5h + 16min));
  /* 2021-06-14 16:42 Asia/Kuala_Lumpur  =  2021-06-14 08:42Z */
  CHECK(isUtc(p.asr, 2021y/June/14d, 8h + 42min));
  /* 2021-06-14 19:25 Asia/Kuala_Lumpur  =  2021-06-14 11:25Z */
  CHECK(isUtc(p.maghrib, 2021y/June/14d, 11h + 25min));
  /* 2021-06-14 20:41 Asia/Kuala_Lumpur  =  2021-06-14 12:41Z */
  CHECK(isUtc(p.isha, 2021y/June/14d, 12h + 41min));
}

TEST_CASE("changing the time for asr with different madhabs") {
  const year_month_day date{2015y/December/1d};
  CalculationParameters params = CalculationMethod::MuslimWorldLeague();
  params.madhab = Madhab::Shafi;
  PrayerTimes p(Coordinates(35.775, -78.6336), date, params);
  /* 2015-12-01 14:42 America/New_York  =  2015-12-01 19:42Z */
  CHECK(isUtc(p.asr, 2015y/December/1d, 19h + 42min));

  params.madhab = Madhab::Hanafi;

  PrayerTimes p2(Coordinates(35.775, -78.6336), date, params);
  /* 2015-12-01 15:22 America/New_York  =  2015-12-01 20:22Z */
  CHECK(isUtc(p2.asr, 2015y/December/1d, 20h + 22min));
}

TEST_CASE("adjusting prayer time with high latitude rule") {
  const year_month_day date{2020y/June/15d};
  CalculationParameters params = CalculationMethod::MuslimWorldLeague();
  const std::string tzid = "Europe/London";
  Coordinates coords(55.983226, -3.216649);

  PrayerTimes p1(coords, date, params);
  /* 2020-06-15 01:14 Europe/London  =  2020-06-15 00:14Z */
  CHECK(isUtc(p1.fajr, 2020y/June/15d, 0h + 14min));
  /* 2020-06-15 04:26 Europe/London  =  2020-06-15 03:26Z */
  CHECK(isUtc(p1.sunrise, 2020y/June/15d, 3h + 26min));
  /* 2020-06-15 13:14 Europe/London  =  2020-06-15 12:14Z */
  CHECK(isUtc(p1.dhuhr, 2020y/June/15d, 12h + 14min));
  /* 2020-06-15 17:46 Europe/London  =  2020-06-15 16:46Z */
  CHECK(isUtc(p1.asr, 2020y/June/15d, 16h + 46min));
  /* 2020-06-15 22:01 Europe/London  =  2020-06-15 21:01Z */
  CHECK(isUtc(p1.maghrib, 2020y/June/15d, 21h + 1min));
  /* 2020-06-16 01:14 Europe/London  =  2020-06-16 00:14Z */
  CHECK(isUtc(p1.isha, 2020y/June/16d, 0h + 14min));

  params.highLatitudeRule = HighLatitudeRule::SeventhOfTheNight;
  PrayerTimes p2(coords, date, params);
  /* 2020-06-15 03:31 Europe/London  =  2020-06-15 02:31Z */
  CHECK(isUtc(p2.fajr, 2020y/June/15d, 2h + 31min));
  /* 2020-06-15 04:26 Europe/London  =  2020-06-15 03:26Z */
  CHECK(isUtc(p2.sunrise, 2020y/June/15d, 3h + 26min));
  /* 2020-06-15 13:14 Europe/London  =  2020-06-15 12:14Z */
  CHECK(isUtc(p2.dhuhr, 2020y/June/15d, 12h + 14min));
  /* 2020-06-15 17:46 Europe/London  =  2020-06-15 16:46Z */
  CHECK(isUtc(p2.asr, 2020y/June/15d, 16h + 46min));
  /* 2020-06-15 22:01 Europe/London  =  2020-06-15 21:01Z */
  CHECK(isUtc(p2.maghrib, 2020y/June/15d, 21h + 1min));
  /* 2020-06-15 22:56 Europe/London  =  2020-06-15 21:56Z */
  CHECK(isUtc(p2.isha, 2020y/June/15d, 21h + 56min));

  params.highLatitudeRule = HighLatitudeRule::TwilightAngle;
  PrayerTimes p3(coords, date, params);
  /* 2020-06-15 02:31 Europe/London  =  2020-06-15 01:31Z */
  CHECK(isUtc(p3.fajr, 2020y/June/15d, 1h + 31min));
  /* 2020-06-15 04:26 Europe/London  =  2020-06-15 03:26Z */
  CHECK(isUtc(p3.sunrise, 2020y/June/15d, 3h + 26min));
  /* 2020-06-15 13:14 Europe/London  =  2020-06-15 12:14Z */
  CHECK(isUtc(p3.dhuhr, 2020y/June/15d, 12h + 14min));
  /* 2020-06-15 17:46 Europe/London  =  2020-06-15 16:46Z */
  CHECK(isUtc(p3.asr, 2020y/June/15d, 16h + 46min));
  /* 2020-06-15 22:01 Europe/London  =  2020-06-15 21:01Z */
  CHECK(isUtc(p3.maghrib, 2020y/June/15d, 21h + 1min));
  /* 2020-06-15 23:50 Europe/London  =  2020-06-15 22:50Z */
  CHECK(isUtc(p3.isha, 2020y/June/15d, 22h + 50min));
}

TEST_SUITE("Moonsighting Committee method with shafaq general") {
  // Values from http://www.moonsighting.com/pray.php
  TEST_CASE("Shafaq general in winter") {
    const year_month_day date{2021y/January/1d};
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::General;
    params.madhab = Madhab::Hanafi;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    /* 2021-01-01 06:16 America/New_York  =  2021-01-01 11:16Z */
    CHECK(isUtc(p.fajr, 2021y/January/1d, 11h + 16min));
    /* 2021-01-01 07:52 America/New_York  =  2021-01-01 12:52Z */
    CHECK(isUtc(p.sunrise, 2021y/January/1d, 12h + 52min));
    /* 2021-01-01 12:28 America/New_York  =  2021-01-01 17:28Z */
    CHECK(isUtc(p.dhuhr, 2021y/January/1d, 17h + 28min));
    /* 2021-01-01 15:12 America/New_York  =  2021-01-01 20:12Z */
    CHECK(isUtc(p.asr, 2021y/January/1d, 20h + 12min));
    /* 2021-01-01 16:57 America/New_York  =  2021-01-01 21:57Z */
    CHECK(isUtc(p.maghrib, 2021y/January/1d, 21h + 57min));
    /* 2021-01-01 18:27 America/New_York  =  2021-01-01 23:27Z */
    CHECK(isUtc(p.isha, 2021y/January/1d, 23h + 27min));
  }

  TEST_CASE("Shafaq general in Spring") {
    const year_month_day date{2021y/April/1d};
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::General;
    params.madhab = Madhab::Hanafi;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    /* 2021-04-01 05:28 America/New_York  =  2021-04-01 09:28Z */
    CHECK(isUtc(p.fajr, 2021y/April/1d, 9h + 28min));
    /* 2021-04-01 07:01 America/New_York  =  2021-04-01 11:01Z */
    CHECK(isUtc(p.sunrise, 2021y/April/1d, 11h + 1min));
    /* 2021-04-01 13:28 America/New_York  =  2021-04-01 17:28Z */
    CHECK(isUtc(p.dhuhr, 2021y/April/1d, 17h + 28min));
    /* 2021-04-01 17:53 America/New_York  =  2021-04-01 21:53Z */
    CHECK(isUtc(p.asr, 2021y/April/1d, 21h + 53min));
    /* 2021-04-01 19:49 America/New_York  =  2021-04-01 23:49Z */
    CHECK(isUtc(p.maghrib, 2021y/April/1d, 23h + 49min));
    /* 2021-04-01 21:01 America/New_York  =  2021-04-02 01:01Z */
    CHECK(isUtc(p.isha, 2021y/April/2d, 1h + 1min));
  }

  TEST_CASE("Shafaq general in Summer") {
    const year_month_day date{2021y/July/1d};
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::General;
    params.madhab = Madhab::Hanafi;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    /* 2021-07-01 03:52 America/New_York  =  2021-07-01 07:52Z */
    CHECK(isUtc(p.fajr, 2021y/July/1d, 7h + 52min));
    /* 2021-07-01 05:42 America/New_York  =  2021-07-01 09:42Z */
    CHECK(isUtc(p.sunrise, 2021y/July/1d, 9h + 42min));
    /* 2021-07-01 13:28 America/New_York  =  2021-07-01 17:28Z */
    CHECK(isUtc(p.dhuhr, 2021y/July/1d, 17h + 28min));
    /* 2021-07-01 18:42 America/New_York  =  2021-07-01 22:42Z */
    CHECK(isUtc(p.asr, 2021y/July/1d, 22h + 42min));
    /* 2021-07-01 21:07 America/New_York  =  2021-07-02 01:07Z */
    CHECK(isUtc(p.maghrib, 2021y/July/2d, 1h + 7min));
    /* 2021-07-01 22:22 America/New_York  =  2021-07-02 02:22Z */
    CHECK(isUtc(p.isha, 2021y/July/2d, 2h + 22min));
  }

  TEST_CASE("Shafaq general in Fall") {
    const year_month_day date{2021y/November/1d};
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::General;
    params.madhab = Madhab::Hanafi;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    /* 2021-11-01 06:22 America/New_York  =  2021-11-01 10:22Z */
    CHECK(isUtc(p.fajr, 2021y/November/1d, 10h + 22min));
    /* 2021-11-01 07:55 America/New_York  =  2021-11-01 11:55Z */
    CHECK(isUtc(p.sunrise, 2021y/November/1d, 11h + 55min));
    /* 2021-11-01 13:08 America/New_York  =  2021-11-01 17:08Z */
    CHECK(isUtc(p.dhuhr, 2021y/November/1d, 17h + 8min));
    /* 2021-11-01 16:26 America/New_York  =  2021-11-01 20:26Z */
    CHECK(isUtc(p.asr, 2021y/November/1d, 20h + 26min));
    /* 2021-11-01 18:13 America/New_York  =  2021-11-01 22:13Z */
    CHECK(isUtc(p.maghrib, 2021y/November/1d, 22h + 13min));
    /* 2021-11-01 19:35 America/New_York  =  2021-11-01 23:35Z */
    CHECK(isUtc(p.isha, 2021y/November/1d, 23h + 35min));
  }
}

TEST_SUITE("Moonsighting Committee method with shafaq ahmer") {
  // Values from http://www.moonsighting.com/pray.php
  TEST_CASE("Shafaq ahmer in winter") {
    const year_month_day date{2021y/January/1d};
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::Ahmer;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    /* 2021-01-01 06:16 America/New_York  =  2021-01-01 11:16Z */
    CHECK(isUtc(p.fajr, 2021y/January/1d, 11h + 16min));
    /* 2021-01-01 07:52 America/New_York  =  2021-01-01 12:52Z */
    CHECK(isUtc(p.sunrise, 2021y/January/1d, 12h + 52min));
    /* 2021-01-01 12:28 America/New_York  =  2021-01-01 17:28Z */
    CHECK(isUtc(p.dhuhr, 2021y/January/1d, 17h + 28min));
    /* 2021-01-01 14:37 America/New_York  =  2021-01-01 19:37Z */
    CHECK(isUtc(p.asr, 2021y/January/1d, 19h + 37min));
    /* 2021-01-01 16:57 America/New_York  =  2021-01-01 21:57Z */
    CHECK(isUtc(p.maghrib, 2021y/January/1d, 21h + 57min));
    /**
     * 2021-01-01 18:07 America/New_York  =  2021-01-01 23:07Z
     * Value from source is 6:08 PM
     */
    CHECK(isUtc(p.isha, 2021y/January/1d, 23h + 7min));
  }

  TEST_CASE("Shafaq ahmer in Spring") {
    const year_month_day date{2021y/April/1d};
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::Ahmer;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    /* 2021-04-01 05:28 America/New_York  =  2021-04-01 09:28Z */
    CHECK(isUtc(p.fajr, 2021y/April/1d, 9h + 28min));
    /* 2021-04-01 07:01 America/New_York  =  2021-04-01 11:01Z */
    CHECK(isUtc(p.sunrise, 2021y/April/1d, 11h + 1min));
    /* 2021-04-01 13:28 America/New_York  =  2021-04-01 17:28Z */
    CHECK(isUtc(p.dhuhr, 2021y/April/1d, 17h + 28min));
    /* 2021-04-01 16:59 America/New_York  =  2021-04-01 20:59Z */
    CHECK(isUtc(p.asr, 2021y/April/1d, 20h + 59min));
    /* 2021-04-01 19:49 America/New_York  =  2021-04-01 23:49Z */
    CHECK(isUtc(p.maghrib, 2021y/April/1d, 23h + 49min));
    /* 2021-04-01 20:45 America/New_York  =  2021-04-02 00:45Z */
    CHECK(isUtc(p.isha, 2021y/April/2d, 0h + 45min));
  }

  TEST_CASE("Shafaq ahmer in Summer") {
    const year_month_day date{2021y/July/1d};
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::Ahmer;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    /* 2021-07-01 03:52 America/New_York  =  2021-07-01 07:52Z */
    CHECK(isUtc(p.fajr, 2021y/July/1d, 7h + 52min));
    /* 2021-07-01 05:42 America/New_York  =  2021-07-01 09:42Z */
    CHECK(isUtc(p.sunrise, 2021y/July/1d, 9h + 42min));
    /* 2021-07-01 13:28 America/New_York  =  2021-07-01 17:28Z */
    CHECK(isUtc(p.dhuhr, 2021y/July/1d, 17h + 28min));
    /* 2021-07-01 17:29 America/New_York  =  2021-07-01 21:29Z */
    CHECK(isUtc(p.asr, 2021y/July/1d, 21h + 29min));
    /* 2021-07-01 21:07 America/New_York  =  2021-07-02 01:07Z */
    CHECK(isUtc(p.maghrib, 2021y/July/2d, 1h + 7min));
    /* 2021-07-01 22:19 America/New_York  =  2021-07-02 02:19Z */
    CHECK(isUtc(p.isha, 2021y/July/2d, 2h + 19min));
  }

  TEST_CASE("Shafaq ahmer in Fall") {
    const year_month_day date{2021y/November/1d};
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::Ahmer;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    /* 2021-11-01 06:22 America/New_York  =  2021-11-01 10:22Z */
    CHECK(isUtc(p.fajr, 2021y/November/1d, 10h + 22min));
    /* 2021-11-01 07:55 America/New_York  =  2021-11-01 11:55Z */
    CHECK(isUtc(p.sunrise, 2021y/November/1d, 11h + 55min));
    /* 2021-11-01 13:08 America/New_York  =  2021-11-01 17:08Z */
    CHECK(isUtc(p.dhuhr, 2021y/November/1d, 17h + 8min));
    /* 2021-11-01 15:45 America/New_York  =  2021-11-01 19:45Z */
    CHECK(isUtc(p.asr, 2021y/November/1d, 19h + 45min));
    /* 2021-11-01 18:13 America/New_York  =  2021-11-01 22:13Z */
    CHECK(isUtc(p.maghrib, 2021y/November/1d, 22h + 13min));
    /* 2021-11-01 19:15 America/New_York  =  2021-11-01 23:15Z */
    CHECK(isUtc(p.isha, 2021y/November/1d, 23h + 15min));
  }
}

/**
 * Only isha is checked here. Shafaq is read in exactly one place,
 * seasonAdjustedEveningTwilight, and PrayerTimes only calls that on the isha
 * path. These cases share their coordinates, date, method and madhab with the
 * shafaq general suite above, so every other time is produced by identical
 * code from identical inputs and is already asserted there.
 */
TEST_SUITE("Moonsighting Committee method with shafaq abyad") {
  // Values from http://www.moonsighting.com/pray.php
  TEST_CASE("Shafaq abyad in winter") {
    const year_month_day date{2021y/January/1d};
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::Abyad;
    params.madhab = Madhab::Hanafi;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    /* 2021-01-01 18:28 America/New_York  =  2021-01-01 23:28Z */
    CHECK(isUtc(p.isha, 2021y/January/1d, 23h + 28min));
  }

  TEST_CASE("Shafaq abyad in Spring") {
    const year_month_day date{2021y/April/1d};
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::Abyad;
    params.madhab = Madhab::Hanafi;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    /* 2021-04-01 21:12 America/New_York  =  2021-04-02 01:12Z */
    CHECK(isUtc(p.isha, 2021y/April/2d, 1h + 12min));
  }

  TEST_CASE("Shafaq abyad in Summer") {
    const year_month_day date{2021y/July/1d};
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::Abyad;
    params.madhab = Madhab::Hanafi;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    /* 2021-07-01 23:17 America/New_York  =  2021-07-02 03:17Z */
    CHECK(isUtc(p.isha, 2021y/July/2d, 3h + 17min));
  }

  TEST_CASE("Shafaq abyad in Fall") {
    const year_month_day date{2021y/November/1d};
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::Abyad;
    params.madhab = Madhab::Hanafi;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    /* 2021-11-01 19:37 America/New_York  =  2021-11-01 23:37Z */
    CHECK(isUtc(p.isha, 2021y/November/1d, 23h + 37min));
  }
}

/* Second part */
TEST_CASE("getting the time for a given prayer") {
  const year_month_day date{2016y/July/1d};
  CalculationParameters params = CalculationMethod::MuslimWorldLeague();
  params.madhab = Madhab::Hanafi;
  params.highLatitudeRule = HighLatitudeRule::TwilightAngle;
  PrayerTimes p(Coordinates(59.9094, 10.7349), date, params);

  CHECK(p.timeForPrayer(Prayer::Fajr) == p.fajr);
  CHECK(p.timeForPrayer(Prayer::Sunrise) == p.sunrise);
  CHECK(p.timeForPrayer(Prayer::Dhuhr) == p.dhuhr);
  CHECK(p.timeForPrayer(Prayer::Asr) == p.asr);
  CHECK(p.timeForPrayer(Prayer::Maghrib) == p.maghrib);
  CHECK(p.timeForPrayer(Prayer::Isha) == p.isha);
  CHECK_FALSE(p.timeForPrayer(Prayer::None).has_value());
}

TEST_CASE("getting the current prayer") {
  const year_month_day date{2015y/September/1d};
  CalculationParameters params = CalculationMethod::Karachi();
  params.madhab = Madhab::Hanafi;
  params.highLatitudeRule = HighLatitudeRule::TwilightAngle;
  PrayerTimes p(Coordinates(33.720817, 73.090032), date, params);

  CHECK(p.currentPrayer(dateByAddingSeconds(p.fajr, -1).value()) == Prayer::None);
  CHECK(p.currentPrayer(p.fajr.value()) == Prayer::Fajr);
  CHECK(p.currentPrayer(dateByAddingSeconds(p.fajr, 1).value()) == Prayer::Fajr);
  CHECK(p.currentPrayer(dateByAddingSeconds(p.sunrise, 1).value()) == Prayer::Sunrise);
  CHECK(p.currentPrayer(dateByAddingSeconds(p.dhuhr, 1).value()) == Prayer::Dhuhr);
  CHECK(p.currentPrayer(dateByAddingSeconds(p.asr, 1).value()) == Prayer::Asr);
  CHECK(p.currentPrayer(dateByAddingSeconds(p.maghrib, 1).value()) == Prayer::Maghrib);
  CHECK(p.currentPrayer(dateByAddingSeconds(p.isha, 1).value()) == Prayer::Isha);
}

TEST_CASE("getting the next prayer") {
  const year_month_day date{2015y/September/1d};
  CalculationParameters params = CalculationMethod::Karachi();
  params.madhab = Madhab::Hanafi;
  params.highLatitudeRule = HighLatitudeRule::TwilightAngle;
  PrayerTimes p(Coordinates(33.720817, 73.090032), date, params);

  CHECK(p.nextPrayer(dateByAddingSeconds(p.fajr, -1).value()) == Prayer::Fajr);
  CHECK(p.nextPrayer(p.fajr.value()) == Prayer::Sunrise);
  CHECK(p.nextPrayer(dateByAddingSeconds(p.fajr, 1).value()) == Prayer::Sunrise);
  CHECK(p.nextPrayer(dateByAddingSeconds(p.sunrise, 1).value()) == Prayer::Dhuhr);
  CHECK(p.nextPrayer(dateByAddingSeconds(p.dhuhr, 1).value()) == Prayer::Asr);
  CHECK(p.nextPrayer(dateByAddingSeconds(p.asr, 1).value()) == Prayer::Maghrib);
  CHECK(p.nextPrayer(dateByAddingSeconds(p.maghrib, 1).value()) == Prayer::Isha);
  CHECK(p.nextPrayer(dateByAddingSeconds(p.isha, 1).value()) == Prayer::None);
}

/**
 * A smoke test, and the only one that runs the no argument overloads and the
 * now() they default to. The check itself is deliberately loose, because the
 * answer moves with the clock. Both coming back as None would mean every time
 * on the day was missing, which cannot happen at this latitude.
 *
 * The ordering work is done by the two fixed date cases above.
 */
TEST_CASE("currentPrayer and nextPrayer default to the current time") {
  const year_month_day date{floor<days>(now())};
  CalculationParameters params = CalculationMethod::Karachi();
  params.madhab = Madhab::Hanafi;
  params.highLatitudeRule = HighLatitudeRule::TwilightAngle;
  PrayerTimes p(Coordinates(33.720817, 73.090032), date, params);
  Prayer current = p.currentPrayer();
  Prayer next = p.nextPrayer();
  CHECK((current != Prayer::None || next != Prayer::None));
}

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

  const year_month_day date{2025y/December/1d};

  PrayerTimes p1(
      Coordinates(42.74674252600066, 177.2401196144623), date, params);
  CHECK(p1.fajr.value() < p1.sunrise.value());
  CHECK(p1.sunrise.value() < p1.dhuhr.value());
  CHECK(p1.dhuhr.value() < p1.asr.value());
  CHECK(p1.asr.value() < p1.maghrib.value());
  CHECK(p1.maghrib.value() < p1.isha.value());

  PrayerTimes p2(
      Coordinates(47.082209457885355, 177.24642294208638), date, params);
  CHECK(p2.fajr.value() < p2.sunrise.value());
  CHECK(p2.sunrise.value() < p2.dhuhr.value());
  CHECK(p2.dhuhr.value() < p2.asr.value());
  CHECK(p2.asr.value() < p2.maghrib.value());
  CHECK(p2.maghrib.value() < p2.isha.value());
}
