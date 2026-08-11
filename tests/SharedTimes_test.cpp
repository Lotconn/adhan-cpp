#include "doctest.h"

#include <adhan/CalculationMethod.hpp>
#include <adhan/Coordinates.hpp>
#include <adhan/DateUtils.hpp>
#include <adhan/HighLatitudeRule.hpp>
#include <adhan/Madhab.hpp>
#include <adhan/PrayerTimes.hpp>

#include "json.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

using json = nlohmann::json;

using namespace Adhan;

namespace {

/**
 * Reads a "YYYY-MM-DD HH:MM" UTC stamp out of a generated fixture.
 *
 * The fixtures under Shared/Times record local wall clock readings and name
 * an IANA zone. The ones under Shared/Times/UTC are those same readings
 * already converted, which is why nothing here has to resolve a zone. See
 * tools/fixtures/generate_utc_fixtures.py.
 */
Instant parseUtc(const std::string &stamp) {
  const int year = std::stoi(stamp.substr(0, 4));
  const int month = std::stoi(stamp.substr(5, 2));
  const int day = std::stoi(stamp.substr(8, 2));
  const int hour = std::stoi(stamp.substr(11, 2));
  const int minute = std::stoi(stamp.substr(14, 2));

  const std::chrono::year_month_day date{
      std::chrono::year{year}, std::chrono::month{static_cast<unsigned>(month)},
      std::chrono::day{static_cast<unsigned>(day)}};

  return std::chrono::sys_days{date} + std::chrono::hours{hour} +
         std::chrono::minutes{minute};
}

/**
 * The day being calculated is a bare calendar date, which is what the
 * library takes. No zone is involved on this side.
 */
std::chrono::year_month_day parseFixtureDate(const std::string &dateStr) {
  return std::chrono::year_month_day{
      std::chrono::year{std::stoi(dateStr.substr(0, 4))},
      std::chrono::month{
          static_cast<unsigned>(std::stoi(dateStr.substr(5, 2)))},
      std::chrono::day{static_cast<unsigned>(std::stoi(dateStr.substr(8, 2)))}};
}

/**
 * Mirrors the custom `toBeWithinRange(comparisonDate, variance)` Jest matcher:
 * passes if actual is within `variance` minutes of expected, inclusive.
 * An absent time never passes.
 */
bool withinRange(
    const OptInstant &actual, const Instant &expected, double varianceMinutes) {
  if (!actual) {
    return false;
  }

  const auto variance = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::duration<double>{varianceMinutes * 60});
  return *actual >= (expected - variance) && *actual <= (expected + variance);
}

/* Mirrors parseParams() from the TS test file */
CalculationParameters parseParams(const json &data) {
  std::string method = data.value("method", std::string());

  CalculationParameters params = [&]() {
    if (method == "MuslimWorldLeague") {
      return CalculationMethod::MuslimWorldLeague();
    }
    if (method == "Egyptian") {
      return CalculationMethod::Egyptian();
    }
    if (method == "Karachi") {
      return CalculationMethod::Karachi();
    }
    if (method == "UmmAlQura") {
      return CalculationMethod::UmmAlQura();
    }
    if (method == "Dubai") {
      return CalculationMethod::Dubai();
    }
    if (method == "MoonsightingCommittee") {
      return CalculationMethod::MoonsightingCommittee();
    }
    if (method == "NorthAmerica") {
      return CalculationMethod::NorthAmerica();
    }
    if (method == "Kuwait") {
      return CalculationMethod::Kuwait();
    }
    if (method == "Qatar") {
      return CalculationMethod::Qatar();
    }
    if (method == "Singapore") {
      return CalculationMethod::Singapore();
    }
    if (method == "Turkey") {
      return CalculationMethod::Turkey();
    }
    if (method == "Tehran") {
      return CalculationMethod::Tehran();
    }
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
  const std::string dir = ADHAN_FIXTURE_DIR;

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

      Coordinates coordinates(
          paramsJson["latitude"].get<double>(),
          paramsJson["longitude"].get<double>());
      CalculationParameters params = parseParams(paramsJson);
      double variance = data.value("variance", 0.0);

      for (const auto &timeEntry : data["times"]) {
        std::string dateStr = timeEntry["date"].get<std::string>();
        PrayerTimes p(coordinates, parseFixtureDate(dateStr), params);

        Instant testFajr = parseUtc(timeEntry["fajr"].get<std::string>());
        Instant testSunrise = parseUtc(timeEntry["sunrise"].get<std::string>());
        Instant testDhuhr = parseUtc(timeEntry["dhuhr"].get<std::string>());
        Instant testAsr = parseUtc(timeEntry["asr"].get<std::string>());
        Instant testMaghrib = parseUtc(timeEntry["maghrib"].get<std::string>());
        Instant testIsha = parseUtc(timeEntry["isha"].get<std::string>());

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
