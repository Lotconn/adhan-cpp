#include "DateUtils.hpp"
#include "HighLatitudeRule.hpp"
#include "doctest.h"

#include "CalculationMethod.hpp"
#include "Coordinates.hpp"
#include "JSDate.hpp"
#include "Madhab.hpp"
#if not defined(ADHAN_USE_CTIME_FALLBACK)
#include "MomentFormat.hpp"
#endif
#include "PrayerTimes.hpp"

using namespace Adhan;

/* ! Feature not a part of our core library ! */
/* All the test suites/cases that use `formatInZone` */
/* Excluded for the fallback build - will fail in unsupported systems */
#if not defined(ADHAN_USE_CTIME_FALLBACK)
TEST_CASE("calculating prayer times") {
  JSDate date(2015, 6, 12);
  CalculationParameters params = CalculationMethod::NorthAmerica();
  params.madhab = Madhab::Hanafi;
  PrayerTimes p(Coordinates(35.775, -78.6336), date, params);

  CHECK(formatInZone(p.fajr, "America/New_York", "h:mm A") == "4:42 AM");
  CHECK(formatInZone(p.sunrise, "America/New_York", "h:mm A") == "6:08 AM");
  CHECK(formatInZone(p.dhuhr, "America/New_York", "h:mm A") == "1:21 PM");
  CHECK(formatInZone(p.asr, "America/New_York", "h:mm A") == "6:22 PM");
  CHECK(formatInZone(p.maghrib, "America/New_York", "h:mm A") == "8:32 PM");
  CHECK(formatInZone(p.isha, "America/New_York", "h:mm A") == "9:57 PM");
  CHECK(formatInZone(p.isha, "America/New_York", "HH:mm") == "21:57");
}

TEST_CASE("using offsets to manually adjust prayer times") {
  JSDate date(2015, 11, 1);
  CalculationParameters params = CalculationMethod::MuslimWorldLeague();
  params.madhab = Madhab::Shafi;
  PrayerTimes p(Coordinates(35.775, -78.6336), date, params);

  CHECK(formatInZone(p.fajr, "America/New_York", "h:mm A") == "5:35 AM");
  CHECK(formatInZone(p.sunrise, "America/New_York", "h:mm A") == "7:06 AM");
  CHECK(formatInZone(p.dhuhr, "America/New_York", "h:mm A") == "12:05 PM");
  CHECK(formatInZone(p.asr, "America/New_York", "h:mm A") == "2:42 PM");
  CHECK(formatInZone(p.maghrib, "America/New_York", "h:mm A") == "5:01 PM");
  CHECK(formatInZone(p.isha, "America/New_York", "h:mm A") == "6:26 PM");

  params.adjustments.fajr = 10;
  params.adjustments.sunrise = 10;
  params.adjustments.dhuhr = 10;
  params.adjustments.asr = 10;
  params.adjustments.maghrib = 10;
  params.adjustments.isha = 10;

  PrayerTimes p2(Coordinates(35.775, -78.6336), date, params);
  CHECK(formatInZone(p2.fajr, "America/New_York", "h:mm A") == "5:45 AM");
  CHECK(formatInZone(p2.sunrise, "America/New_York", "h:mm A") == "7:16 AM");
  CHECK(formatInZone(p2.dhuhr, "America/New_York", "h:mm A") == "12:15 PM");
  CHECK(formatInZone(p2.asr, "America/New_York", "h:mm A") == "2:52 PM");
  CHECK(formatInZone(p2.maghrib, "America/New_York", "h:mm A") == "5:11 PM");
  CHECK(formatInZone(p2.isha, "America/New_York", "h:mm A") == "6:36 PM");
}

TEST_CASE("calculating prayer times using the Moonsighting Committee "
          "calculation method") {
  // Values from http://www.moonsighting.com/pray.php
  JSDate date(2016, 0, 31);
  PrayerTimes p(Coordinates(35.775, -78.6336), date,
                CalculationMethod::MoonsightingCommittee());

  CHECK(formatInZone(p.fajr, "America/New_York", "h:mm A") == "5:48 AM");
  CHECK(formatInZone(p.sunrise, "America/New_York", "h:mm A") == "7:16 AM");
  CHECK(formatInZone(p.dhuhr, "America/New_York", "h:mm A") == "12:33 PM");
  CHECK(formatInZone(p.asr, "America/New_York", "h:mm A") == "3:20 PM");
  CHECK(formatInZone(p.maghrib, "America/New_York", "h:mm A") == "5:43 PM");
  CHECK(formatInZone(p.isha, "America/New_York", "h:mm A") == "7:05 PM");
}

TEST_CASE("calculating Moonsighting Committee prayer times at a high latitude "
          "location") {
  // Values from http://www.moonsighting.com/pray.php
  JSDate date(2016, 0, 1);
  CalculationParameters params = CalculationMethod::MoonsightingCommittee();
  params.madhab = Madhab::Hanafi;
  PrayerTimes p(Coordinates(59.9094, 10.7349), date, params);

  CHECK(formatInZone(p.fajr, "Europe/Oslo", "h:mm A") == "7:34 AM");
  CHECK(formatInZone(p.sunrise, "Europe/Oslo", "h:mm A") == "9:19 AM");
  CHECK(formatInZone(p.dhuhr, "Europe/Oslo", "h:mm A") == "12:25 PM");
  CHECK(formatInZone(p.asr, "Europe/Oslo", "h:mm A") == "1:36 PM");
  CHECK(formatInZone(p.maghrib, "Europe/Oslo", "h:mm A") == "3:25 PM");
  CHECK(formatInZone(p.isha, "Europe/Oslo", "h:mm A") == "5:02 PM");
}

TEST_CASE("calculating times for turkey method") {
  // values from
  // https://namazvakitleri.diyanet.gov.tr/en-US/9541/prayer-time-for-istanbul
  JSDate date(2020, 3, 16);
  CalculationParameters params = CalculationMethod::Turkey();
  PrayerTimes p(Coordinates(41.005616, 28.97638), date, params);

  CHECK(formatInZone(p.fajr, "Europe/Istanbul", "h:mm A") == "4:44 AM");
  CHECK(formatInZone(p.sunrise, "Europe/Istanbul", "h:mm A") == "6:16 AM");
  CHECK(formatInZone(p.dhuhr, "Europe/Istanbul", "h:mm A") == "1:09 PM");
  CHECK(formatInZone(p.asr, "Europe/Istanbul", "h:mm A") ==
        "4:53 PM"); // original time 4:52 PM
  CHECK(formatInZone(p.maghrib, "Europe/Istanbul", "h:mm A") == "7:52 PM");
  CHECK(formatInZone(p.isha, "Europe/Istanbul", "h:mm A") ==
        "9:19 PM"); // original time 9:18 PM
}

TEST_CASE("calculating times for the egyptian method") {
  JSDate date(2020, 0, 1);
  CalculationParameters params = CalculationMethod::Egyptian();
  PrayerTimes p(Coordinates(30.028703, 31.249528), date, params);

  CHECK(formatInZone(p.fajr, "Africa/Cairo", "h:mm A") == "5:18 AM");
  CHECK(formatInZone(p.sunrise, "Africa/Cairo", "h:mm A") == "6:51 AM");
  CHECK(formatInZone(p.dhuhr, "Africa/Cairo", "h:mm A") == "11:59 AM");
  CHECK(formatInZone(p.asr, "Africa/Cairo", "h:mm A") == "2:47 PM");
  CHECK(formatInZone(p.maghrib, "Africa/Cairo", "h:mm A") == "5:06 PM");
  CHECK(formatInZone(p.isha, "Africa/Cairo", "h:mm A") == "6:29 PM");
}

TEST_CASE("calculating times for the singapore method") {
  JSDate date(2021, 5, 14);
  CalculationParameters params = CalculationMethod::Singapore();
  PrayerTimes p(Coordinates(3.7333333333, 101.3833333333), date, params);

  CHECK(formatInZone(p.fajr, "Asia/Kuala_Lumpur", "h:mm A") == "5:41 AM");
  CHECK(formatInZone(p.sunrise, "Asia/Kuala_Lumpur", "h:mm A") == "7:05 AM");
  CHECK(formatInZone(p.dhuhr, "Asia/Kuala_Lumpur", "h:mm A") == "1:16 PM");
  CHECK(formatInZone(p.asr, "Asia/Kuala_Lumpur", "h:mm A") == "4:42 PM");
  CHECK(formatInZone(p.maghrib, "Asia/Kuala_Lumpur", "h:mm A") == "7:25 PM");
  CHECK(formatInZone(p.isha, "Asia/Kuala_Lumpur", "h:mm A") == "8:41 PM");
}

TEST_CASE("changing the time for asr with different madhabs") {
  JSDate date(2015, 11, 1);
  CalculationParameters params = CalculationMethod::MuslimWorldLeague();
  params.madhab = Madhab::Shafi;
  PrayerTimes p(Coordinates(35.775, -78.6336), date, params);
  CHECK(formatInZone(p.asr, "America/New_York", "h:mm A") == "2:42 PM");

  params.madhab = Madhab::Hanafi;

  PrayerTimes p2(Coordinates(35.775, -78.6336), date, params);
  CHECK(formatInZone(p2.asr, "America/New_York", "h:mm A") == "3:22 PM");
}

TEST_CASE("adjusting prayer time with high latitude rule") {
  JSDate date(2020, 5, 15);
  CalculationParameters params = CalculationMethod::MuslimWorldLeague();
  const std::string tzid = "Europe/London";
  Coordinates coords(55.983226, -3.216649);

  PrayerTimes p1(coords, date, params);
  CHECK(formatInZone(p1.fajr, tzid, "h:mm A") == "1:14 AM");
  CHECK(formatInZone(p1.sunrise, tzid, "h:mm A") == "4:26 AM");
  CHECK(formatInZone(p1.dhuhr, tzid, "h:mm A") == "1:14 PM");
  CHECK(formatInZone(p1.asr, tzid, "h:mm A") == "5:46 PM");
  CHECK(formatInZone(p1.maghrib, tzid, "h:mm A") == "10:01 PM");
  CHECK(formatInZone(p1.isha, tzid, "h:mm A") == "1:14 AM");

  params.highLatitudeRule = HighLatitudeRule::SeventhOfTheNight;
  PrayerTimes p2(coords, date, params);
  CHECK(formatInZone(p2.fajr, tzid, "h:mm A") == "3:31 AM");
  CHECK(formatInZone(p2.sunrise, tzid, "h:mm A") == "4:26 AM");
  CHECK(formatInZone(p2.dhuhr, tzid, "h:mm A") == "1:14 PM");
  CHECK(formatInZone(p2.asr, tzid, "h:mm A") == "5:46 PM");
  CHECK(formatInZone(p2.maghrib, tzid, "h:mm A") == "10:01 PM");
  CHECK(formatInZone(p2.isha, tzid, "h:mm A") == "10:56 PM");

  params.highLatitudeRule = HighLatitudeRule::TwilightAngle;
  PrayerTimes p3(coords, date, params);
  CHECK(formatInZone(p3.fajr, tzid, "h:mm A") == "2:31 AM");
  CHECK(formatInZone(p3.sunrise, tzid, "h:mm A") == "4:26 AM");
  CHECK(formatInZone(p3.dhuhr, tzid, "h:mm A") == "1:14 PM");
  CHECK(formatInZone(p3.asr, tzid, "h:mm A") == "5:46 PM");
  CHECK(formatInZone(p3.maghrib, tzid, "h:mm A") == "10:01 PM");
  CHECK(formatInZone(p3.isha, tzid, "h:mm A") == "11:50 PM");
}

TEST_SUITE("Moonsighting Committee method with shafaq general") {
  // Values from http://www.moonsighting.com/pray.php
  TEST_CASE("Shafaq general in winter") {
    JSDate date(2021, 0, 1);
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::General;
    params.madhab = Madhab::Hanafi;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    CHECK(formatInZone(p.fajr, "America/New_York", "h:mm A") == "6:16 AM");
    CHECK(formatInZone(p.sunrise, "America/New_York", "h:mm A") == "7:52 AM");
    CHECK(formatInZone(p.dhuhr, "America/New_York", "h:mm A") == "12:28 PM");
    CHECK(formatInZone(p.asr, "America/New_York", "h:mm A") == "3:12 PM");
    CHECK(formatInZone(p.maghrib, "America/New_York", "h:mm A") == "4:57 PM");
    CHECK(formatInZone(p.isha, "America/New_York", "h:mm A") == "6:27 PM");
  }

  TEST_CASE("Shafaq general in Spring") {
    JSDate date(2021, 3, 1);
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::General;
    params.madhab = Madhab::Hanafi;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    CHECK(formatInZone(p.fajr, "America/New_York", "h:mm A") == "5:28 AM");
    CHECK(formatInZone(p.sunrise, "America/New_York", "h:mm A") == "7:01 AM");
    CHECK(formatInZone(p.dhuhr, "America/New_York", "h:mm A") == "1:28 PM");
    CHECK(formatInZone(p.asr, "America/New_York", "h:mm A") == "5:53 PM");
    CHECK(formatInZone(p.maghrib, "America/New_York", "h:mm A") == "7:49 PM");
    CHECK(formatInZone(p.isha, "America/New_York", "h:mm A") == "9:01 PM");
  }

  TEST_CASE("Shafaq general in Summer") {
    JSDate date(2021, 6, 1);
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::General;
    params.madhab = Madhab::Hanafi;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    CHECK(formatInZone(p.fajr, "America/New_York", "h:mm A") == "3:52 AM");
    CHECK(formatInZone(p.sunrise, "America/New_York", "h:mm A") == "5:42 AM");
    CHECK(formatInZone(p.dhuhr, "America/New_York", "h:mm A") == "1:28 PM");
    CHECK(formatInZone(p.asr, "America/New_York", "h:mm A") == "6:42 PM");
    CHECK(formatInZone(p.maghrib, "America/New_York", "h:mm A") == "9:07 PM");
    CHECK(formatInZone(p.isha, "America/New_York", "h:mm A") == "10:22 PM");
  }

  TEST_CASE("Shafaq general in Fall") {
    JSDate date(2021, 10, 1);
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::General;
    params.madhab = Madhab::Hanafi;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    CHECK(formatInZone(p.fajr, "America/New_York", "h:mm A") == "6:22 AM");
    CHECK(formatInZone(p.sunrise, "America/New_York", "h:mm A") == "7:55 AM");
    CHECK(formatInZone(p.dhuhr, "America/New_York", "h:mm A") == "1:08 PM");
    CHECK(formatInZone(p.asr, "America/New_York", "h:mm A") == "4:26 PM");
    CHECK(formatInZone(p.maghrib, "America/New_York", "h:mm A") == "6:13 PM");
    CHECK(formatInZone(p.isha, "America/New_York", "h:mm A") == "7:35 PM");
  }
}

TEST_SUITE("Moonsighting Committee method with shafaq ahmer") {
  // Values from http://www.moonsighting.com/pray.php
  TEST_CASE("Shafaq ahmer in winter") {
    JSDate date(2021, 0, 1);
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::Ahmer;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    CHECK(formatInZone(p.fajr, "America/New_York", "h:mm A") == "6:16 AM");
    CHECK(formatInZone(p.sunrise, "America/New_York", "h:mm A") == "7:52 AM");
    CHECK(formatInZone(p.dhuhr, "America/New_York", "h:mm A") == "12:28 PM");
    CHECK(formatInZone(p.asr, "America/New_York", "h:mm A") == "2:37 PM");
    CHECK(formatInZone(p.maghrib, "America/New_York", "h:mm A") == "4:57 PM");
    CHECK(formatInZone(p.isha, "America/New_York", "h:mm A") ==
          "6:07 PM"); // value from source is 6:08 PM
  }

  TEST_CASE("Shafaq ahmer in Spring") {
    JSDate date(2021, 3, 1);
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::Ahmer;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    CHECK(formatInZone(p.fajr, "America/New_York", "h:mm A") == "5:28 AM");
    CHECK(formatInZone(p.sunrise, "America/New_York", "h:mm A") == "7:01 AM");
    CHECK(formatInZone(p.dhuhr, "America/New_York", "h:mm A") == "1:28 PM");
    CHECK(formatInZone(p.asr, "America/New_York", "h:mm A") == "4:59 PM");
    CHECK(formatInZone(p.maghrib, "America/New_York", "h:mm A") == "7:49 PM");
    CHECK(formatInZone(p.isha, "America/New_York", "h:mm A") == "8:45 PM");
  }

  TEST_CASE("Shafaq ahmer in Summer") {
    JSDate date(2021, 6, 1);
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::Ahmer;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    CHECK(formatInZone(p.fajr, "America/New_York", "h:mm A") == "3:52 AM");
    CHECK(formatInZone(p.sunrise, "America/New_York", "h:mm A") == "5:42 AM");
    CHECK(formatInZone(p.dhuhr, "America/New_York", "h:mm A") == "1:28 PM");
    CHECK(formatInZone(p.asr, "America/New_York", "h:mm A") == "5:29 PM");
    CHECK(formatInZone(p.maghrib, "America/New_York", "h:mm A") == "9:07 PM");
    CHECK(formatInZone(p.isha, "America/New_York", "h:mm A") == "10:19 PM");
  }

  TEST_CASE("Shafaq ahmer in Fall") {
    JSDate date(2021, 10, 1);
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::Ahmer;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    CHECK(formatInZone(p.fajr, "America/New_York", "h:mm A") == "6:22 AM");
    CHECK(formatInZone(p.sunrise, "America/New_York", "h:mm A") == "7:55 AM");
    CHECK(formatInZone(p.dhuhr, "America/New_York", "h:mm A") == "1:08 PM");
    CHECK(formatInZone(p.asr, "America/New_York", "h:mm A") == "3:45 PM");
    CHECK(formatInZone(p.maghrib, "America/New_York", "h:mm A") == "6:13 PM");
    CHECK(formatInZone(p.isha, "America/New_York", "h:mm A") == "7:15 PM");
  }
}

TEST_SUITE("Moonsighting Committee method with shafaq abyad") {
  // Values from http://www.moonsighting.com/pray.php
  TEST_CASE("Shafaq abyad in winter") {
    JSDate date(2021, 0, 1);
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::Abyad;
    params.madhab = Madhab::Hanafi;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    CHECK(formatInZone(p.fajr, "America/New_York", "h:mm A") == "6:16 AM");
    CHECK(formatInZone(p.sunrise, "America/New_York", "h:mm A") == "7:52 AM");
    CHECK(formatInZone(p.dhuhr, "America/New_York", "h:mm A") == "12:28 PM");
    CHECK(formatInZone(p.asr, "America/New_York", "h:mm A") == "3:12 PM");
    CHECK(formatInZone(p.maghrib, "America/New_York", "h:mm A") == "4:57 PM");
    CHECK(formatInZone(p.isha, "America/New_York", "h:mm A") == "6:28 PM");
  }

  TEST_CASE("Shafaq abyad in Spring") {
    JSDate date(2021, 3, 1);
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::Abyad;
    params.madhab = Madhab::Hanafi;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    CHECK(formatInZone(p.fajr, "America/New_York", "h:mm A") == "5:28 AM");
    CHECK(formatInZone(p.sunrise, "America/New_York", "h:mm A") == "7:01 AM");
    CHECK(formatInZone(p.dhuhr, "America/New_York", "h:mm A") == "1:28 PM");
    CHECK(formatInZone(p.asr, "America/New_York", "h:mm A") == "5:53 PM");
    CHECK(formatInZone(p.maghrib, "America/New_York", "h:mm A") == "7:49 PM");
    CHECK(formatInZone(p.isha, "America/New_York", "h:mm A") == "9:12 PM");
  }

  TEST_CASE("Shafaq abyad in Summer") {
    JSDate date(2021, 6, 1);
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::Abyad;
    params.madhab = Madhab::Hanafi;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    CHECK(formatInZone(p.fajr, "America/New_York", "h:mm A") == "3:52 AM");
    CHECK(formatInZone(p.sunrise, "America/New_York", "h:mm A") == "5:42 AM");
    CHECK(formatInZone(p.dhuhr, "America/New_York", "h:mm A") == "1:28 PM");
    CHECK(formatInZone(p.asr, "America/New_York", "h:mm A") == "6:42 PM");
    CHECK(formatInZone(p.maghrib, "America/New_York", "h:mm A") == "9:07 PM");
    CHECK(formatInZone(p.isha, "America/New_York", "h:mm A") == "11:17 PM");
  }

  TEST_CASE("Shafaq abyad in Fall") {
    JSDate date(2021, 10, 1);
    CalculationParameters params = CalculationMethod::MoonsightingCommittee();
    params.shafaq = Shafaq::Abyad;
    params.madhab = Madhab::Hanafi;
    PrayerTimes p(Coordinates(43.494, -79.844), date, params);
    CHECK(formatInZone(p.fajr, "America/New_York", "h:mm A") == "6:22 AM");
    CHECK(formatInZone(p.sunrise, "America/New_York", "h:mm A") == "7:55 AM");
    CHECK(formatInZone(p.dhuhr, "America/New_York", "h:mm A") == "1:08 PM");
    CHECK(formatInZone(p.asr, "America/New_York", "h:mm A") == "4:26 PM");
    CHECK(formatInZone(p.maghrib, "America/New_York", "h:mm A") == "6:13 PM");
    CHECK(formatInZone(p.isha, "America/New_York", "h:mm A") == "7:37 PM");
  }
}
#endif

/* Second part */
TEST_CASE("getting the time for a given prayer") {
  JSDate date(2016, 6, 1);
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
  JSDate date(2015, 8, 1);
  CalculationParameters params = CalculationMethod::Karachi();
  params.madhab = Madhab::Hanafi;
  params.highLatitudeRule = HighLatitudeRule::TwilightAngle;
  PrayerTimes p(Coordinates(33.720817, 73.090032), date, params);

  CHECK(p.currentPrayer(dateByAddingSeconds(p.fajr, -1)) == Prayer::None);
  CHECK(p.currentPrayer(p.fajr) == Prayer::Fajr);
  CHECK(p.currentPrayer(dateByAddingSeconds(p.fajr, 1)) == Prayer::Fajr);
  CHECK(p.currentPrayer(dateByAddingSeconds(p.sunrise, 1)) == Prayer::Sunrise);
  CHECK(p.currentPrayer(dateByAddingSeconds(p.dhuhr, 1)) == Prayer::Dhuhr);
  CHECK(p.currentPrayer(dateByAddingSeconds(p.asr, 1)) == Prayer::Asr);
  CHECK(p.currentPrayer(dateByAddingSeconds(p.maghrib, 1)) == Prayer::Maghrib);
  CHECK(p.currentPrayer(dateByAddingSeconds(p.isha, 1)) == Prayer::Isha);
}

TEST_CASE("getting the next prayer") {
  JSDate date(2015, 8, 1);
  CalculationParameters params = CalculationMethod::Karachi();
  params.madhab = Madhab::Hanafi;
  params.highLatitudeRule = HighLatitudeRule::TwilightAngle;
  PrayerTimes p(Coordinates(33.720817, 73.090032), date, params);

  CHECK(p.nextPrayer(dateByAddingSeconds(p.fajr, -1)) == Prayer::Fajr);
  CHECK(p.nextPrayer(p.fajr) == Prayer::Sunrise);
  CHECK(p.nextPrayer(dateByAddingSeconds(p.fajr, 1)) == Prayer::Sunrise);
  CHECK(p.nextPrayer(dateByAddingSeconds(p.sunrise, 1)) == Prayer::Dhuhr);
  CHECK(p.nextPrayer(dateByAddingSeconds(p.dhuhr, 1)) == Prayer::Asr);
  CHECK(p.nextPrayer(dateByAddingSeconds(p.asr, 1)) == Prayer::Maghrib);
  CHECK(p.nextPrayer(dateByAddingSeconds(p.maghrib, 1)) == Prayer::Isha);
  CHECK(p.nextPrayer(dateByAddingSeconds(p.isha, 1)) == Prayer::None);
}

TEST_CASE("getting the current next prayer") {
  JSDate date = JSDate::now();
  CalculationParameters params = CalculationMethod::Karachi();
  params.madhab = Madhab::Hanafi;
  params.highLatitudeRule = HighLatitudeRule::TwilightAngle;
  PrayerTimes p(Coordinates(33.720817, 73.090032), date, params);
  Prayer current = p.currentPrayer();
  Prayer next = p.nextPrayer();
  CHECK((current != Prayer::None || next != Prayer::None));
}

TEST_CASE("getting the madhab shadow length") {
  CHECK(shadow_length(Madhab::Shafi) == 1);
  CHECK(shadow_length(Madhab::Hanafi) == 2);
  CHECK_THROWS(shadow_length(static_cast<Madhab>(999)));
}

TEST_CASE("getting recommended high latitude rule") {
  Coordinates coords1(45.983226, -3.216649);
  CHECK(recommended(coords1) == HighLatitudeRule::MiddleOfTheNight);

  Coordinates coords2(48.983226, -3.216649);
  CHECK(recommended(coords2) == HighLatitudeRule::SeventhOfTheNight);
}

/* Third part */