#include "doctest.h"

#include <CalculationMethod.hpp>
#include <Coordinates.hpp>
#include <HighLatitudeRule.hpp>
#include <JSDate.hpp>
#include <Madhab.hpp>
#include <PrayerTimes.hpp>

#include <json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

/**
 * TODO: If similar JSON reading is needed elsewhere, we can consider
 * refactoring the `ParsedDate` and similar JSON date parsing logic
 * into a different module for easier re-use. But for now, this is fine,
 * as this JSON date parsing logic does not bleed into another test.
 */

using json = nlohmann::json;

using namespace adhan;

namespace {

/**
 * date/time string parsing helpers, mirroring moment's
 * format strings used in the TS test
 */

struct ParsedDate {
  int year;
  int month; // 1-indexed, as it appears in a "YYYY-MM-DD" string
  int day;
};

/* Parses a fixed-width "YYYY-MM-DD" string */
ParsedDate parseDateStr(const std::string &s) {
  return {
      std::stoi(s.substr(0, 4)),
      std::stoi(s.substr(5, 2)),
      std::stoi(s.substr(8, 2)),
  };
}

struct ParsedTime {
  int hour24;
  int minute;
};

/* Parses a "h:mm A" string (non-padded hour, e.g. "6:33 AM" or "12:57 PM") */
ParsedTime parseTimeOfDay(const std::string &s) {
  size_t colonPos = s.find(':');
  size_t spacePos = s.find(' ', colonPos);

  int hour12 = std::stoi(s.substr(0, colonPos));
  int minute = std::stoi(s.substr(colonPos + 1, spacePos - colonPos - 1));
  std::string ampm = s.substr(spacePos + 1);

  int hour24 = hour12 % 12;
  if (ampm == "PM") {
    hour24 += 12;
  }
  return {hour24, minute};
}

/**
 * Mirrors `moment(dateStr, 'YYYY-MM-DD').toDate()` — parsed as local
 * (system) time at midnight, with no explicit time zone. This matches
 * JSDate's own local-time constructor semantics directly.
 */
JSDate parseLocalDate(const std::string &dateStr) {
  ParsedDate d = parseDateStr(dateStr);
  return JSDate(d.year, d.month - 1, d.day);
}
/**
 * Mirrors `moment.tz(dateStr + ' ' + timeStr, 'YYYY-MM-DD h:mm A',
 * tzName).toDate()` — parses the date/time as wall-clock time in the given
 * named zone.
 */
JSDate parseInZone(const std::string &tzName, const std::string &dateStr,
                   const std::string &timeStr) {
  using namespace std::chrono;

  ParsedDate d = parseDateStr(dateStr);
  ParsedTime t = parseTimeOfDay(timeStr);

  auto localTime =
      local_days{std::chrono::year{d.year} /
                 std::chrono::month{static_cast<unsigned>(d.month)} /
                 std::chrono::day{static_cast<unsigned>(d.day)}} +
      std::chrono::hours{t.hour24} + std::chrono::minutes{t.minute};

  const time_zone *zone = locate_zone(tzName);
  zoned_time<system_clock::duration> zt{zone, localTime, choose::earliest};
  return JSDate(zt.get_sys_time());
}

/**
 * Mirrors the custom `toBeWithinRange(comparisonDate, variance)` Jest matcher:
 * passes if actual is within `variance` minutes of expected, inclusive.
 */
bool withinRange(const JSDate &actual, const JSDate &expected,
                 double varianceMinutes) {
  long long actualMs = actual.getTime();
  long long expectedMs = expected.getTime();
  long long varianceMs = static_cast<long long>(varianceMinutes * 60 * 1000);
  return actualMs >= (expectedMs - varianceMs) &&
         actualMs <= (expectedMs + varianceMs);
}

/* Mirrors parseParams() from the TS test file */
CalculationParameters parseParams(const json &data) {
  std::string method = data.value("method", std::string());

  CalculationParameters params = [&]() {
    if (method == "MuslimWorldLeague")
      return CalculationMethod::MuslimWorldLeague();
    if (method == "Egyptian")
      return CalculationMethod::Egyptian();
    if (method == "Karachi")
      return CalculationMethod::Karachi();
    if (method == "UmmAlQura")
      return CalculationMethod::UmmAlQura();
    if (method == "Dubai")
      return CalculationMethod::Dubai();
    if (method == "MoonsightingCommittee")
      return CalculationMethod::MoonsightingCommittee();
    if (method == "NorthAmerica")
      return CalculationMethod::NorthAmerica();
    if (method == "Kuwait")
      return CalculationMethod::Kuwait();
    if (method == "Qatar")
      return CalculationMethod::Qatar();
    if (method == "Singapore")
      return CalculationMethod::Singapore();
    if (method == "Turkey")
      return CalculationMethod::Turkey();
    if (method == "Tehran")
      return CalculationMethod::Tehran();
    return CalculationMethod::Other();
  }();

  std::string madhab = data.value("madhab", std::string());
  if (madhab == "Shafi") {
    params.madhab = Madhab::Shafi;
  } else if (madhab == "Hanafi") {
    params.madhab = Madhab::Hanafi;
  }

  std::string highLatRule = data.value("highLatitudeRule", std::string());
  if (highLatRule == "SeventhOfTheNight") {
    params.highLatitudeRule = HighLatitudeRule::SeventhOfTheNight;
  } else if (highLatRule == "TwilightAngle") {
    params.highLatitudeRule = HighLatitudeRule::TwilightAngle;
  } else {
    params.highLatitudeRule = HighLatitudeRule::MiddleOfTheNight;
  }

  return params;
}

} // namespace

TEST_CASE("compare calculated times against the shared prayer time fixtures") {
  const std::string dir = "tests/Shared/Times";

  for (const auto &entry : std::filesystem::directory_iterator(dir)) {
    if (!entry.is_regular_file() || entry.path().extension() != ".json") {
      continue;
    }

    std::string filename = entry.path().filename().string();

    SUBCASE(filename.c_str()) {
      std::ifstream f(entry.path());
      REQUIRE(f.is_open());

      json data = json::parse(f);
      const json &paramsJson = data["params"];

      Coordinates coordinates(paramsJson["latitude"].get<double>(),
                              paramsJson["longitude"].get<double>());
      CalculationParameters params = parseParams(paramsJson);
      double variance = data.value("variance", 0.0);
      std::string timezone = paramsJson["timezone"].get<std::string>();

      for (const auto &timeEntry : data["times"]) {
        std::string dateStr = timeEntry["date"].get<std::string>();
        JSDate date = parseLocalDate(dateStr);
        PrayerTimes p(coordinates, date, params);

        JSDate testFajr = parseInZone(timezone, dateStr,
                                      timeEntry["fajr"].get<std::string>());
        JSDate testSunrise = parseInZone(
            timezone, dateStr, timeEntry["sunrise"].get<std::string>());
        JSDate testDhuhr = parseInZone(timezone, dateStr,
                                       timeEntry["dhuhr"].get<std::string>());
        JSDate testAsr =
            parseInZone(timezone, dateStr, timeEntry["asr"].get<std::string>());
        JSDate testMaghrib = parseInZone(
            timezone, dateStr, timeEntry["maghrib"].get<std::string>());
        JSDate testIsha = parseInZone(timezone, dateStr,
                                      timeEntry["isha"].get<std::string>());

        CHECK(withinRange(p.fajr, testFajr, variance));
        CHECK(withinRange(p.sunrise, testSunrise, variance));
        CHECK(withinRange(p.dhuhr, testDhuhr, variance));
        CHECK(withinRange(p.asr, testAsr, variance));
        CHECK(withinRange(p.maghrib, testMaghrib, variance));
        CHECK(withinRange(p.isha, testIsha, variance));
      }
    }
  }
}