/**
 * Adhan C++ CLI Demo
 *
 * Exercises the full public API of the adhan library: prayer time
 * calculation, Sunnah times, Qibla direction, date utilities, and math
 * utilities. Modeled on the usage patterns shown in the adhan-js README.
 *
 * Usage:
 *   adhan_cli <latitude> <longitude> [date=YYYY-MM-DD] [method] [madhab]
 *             [highLatitudeRule] [shafaq] [rounding] [polarCircleResolution]
 *
 * Example:
 *   adhan_cli 35.78056 -78.6389 2026-01-01 NorthAmerica Hanafi \
 *             TwilightAngle General Nearest Unresolved
 *
 * All arguments after latitude/longitude are optional and take sensible
 * defaults matching the library's own defaults.
 */

#include <adhan/CalculationMethod.hpp>
#include <adhan/CalculationParameters.hpp>
#include <adhan/Coordinates.hpp>
#include <adhan/DateTime.hpp>
#include <adhan/HighLatitudeRule.hpp>
#include <adhan/Madhab.hpp>
#include <adhan/PolarCircleResolution.hpp>
#include <adhan/Prayer.hpp>
#include <adhan/PrayerTimes.hpp>
#include <adhan/Qibla.hpp>
#include <adhan/Rounding.hpp>
#include <adhan/Shafaq.hpp>
#include <adhan/SunnahTimes.hpp>

#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

using namespace Adhan;

namespace {

/**
 * ---------------------------------------------------------------------
 * Small formatting helpers (production-safe: UTC only, no named zones.
 * Named-zone formatting is test-only infrastructure, not shipped).
 * ---------------------------------------------------------------------
 */
std::string pad2(int v) {
  std::ostringstream oss;
  oss << std::setw(2) << std::setfill('0') << v;
  return oss.str();
}

std::string formatUtc(const DateTime &date) {
  if (!date.isValid()) {
    return "(invalid — likely a polar location/date needing resolution)";
  }
  std::ostringstream oss;
  oss << date.getUTCFullYear() << "-" << pad2(date.getUTCMonth() + 1) << "-"
      << pad2(date.getUTCDate()) << " " << pad2(date.getUTCHours()) << ":"
      << pad2(date.getUTCMinutes()) << ":" << pad2(date.getUTCSeconds())
      << " UTC";
  return oss.str();
}

/**
 * ---------------------------------------------------------------------
 * Argument -> enum/parameter mapping, mirroring the CalculationMethod
 * dispatch shown in the library's own test suite.
 * ---------------------------------------------------------------------
 */
CalculationParameters resolveMethod(const std::string &method) {
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
}

Madhab resolveMadhab(const std::string &s) {
  return s == "Hanafi" ? Madhab::Hanafi : Madhab::Shafi;
}

HighLatitudeRule resolveHighLatRule(const std::string &s) {
  if (s == "SeventhOfTheNight")
    return HighLatitudeRule::SeventhOfTheNight;
  if (s == "TwilightAngle")
    return HighLatitudeRule::TwilightAngle;
  return HighLatitudeRule::MiddleOfTheNight;
}

Shafaq resolveShafaq(const std::string &s) {
  if (s == "Ahmer")
    return Shafaq::Ahmer;
  if (s == "Abyad")
    return Shafaq::Abyad;
  return Shafaq::General;
}

Rounding resolveRounding(const std::string &s) {
  if (s == "Up")
    return Rounding::Up;
  if (s == "None")
    return Rounding::None;
  return Rounding::Nearest;
}

PolarCircleResolution resolvePolar(const std::string &s) {
  if (s == "AqrabBalad")
    return PolarCircleResolution::AqrabBalad;
  if (s == "AqrabYaum")
    return PolarCircleResolution::AqrabYaum;
  return PolarCircleResolution::Unresolved;
}

DateTime parseDateArg(const std::string &s) {
  // Expects "YYYY-MM-DD"
  int year = std::stoi(s.substr(0, 4));
  int month = std::stoi(s.substr(5, 2)); // 1-indexed as typed by the user
  int day = std::stoi(s.substr(8, 2));
  return DateTime(year, month - 1,
                  day); // DateTime's month is 0-indexed, like JS
}

std::string prayerName(Prayer p) {
  return std::string(PrayerUtils::to_string(p));
}

void printUsage(const char *progName) {
  std::cout
      << "Usage: " << progName
      << " <latitude> <longitude> [date=YYYY-MM-DD] [method] [madhab]"
         " [highLatitudeRule] [shafaq] [rounding] [polarCircleResolution]\n\n"
      << "  method:            MuslimWorldLeague | Egyptian | Karachi | "
         "UmmAlQura |\n"
      << "                     Dubai | MoonsightingCommittee | NorthAmerica | "
         "Kuwait |\n"
      << "                     Qatar | Singapore | Turkey | Tehran | Other "
         "(default)\n"
      << "  madhab:            Shafi (default) | Hanafi\n"
      << "  highLatitudeRule:  MiddleOfTheNight (default) | SeventhOfTheNight "
         "| TwilightAngle\n"
      << "  shafaq:            General (default) | Ahmer | Abyad\n"
      << "  rounding:          Nearest (default) | Up | None\n"
      << "  polarCircleResolution: Unresolved (default) | AqrabBalad | "
         "AqrabYaum\n\n"
      << "Example:\n  " << progName
      << " 23.775787 90.368047 2026-07-24 MuslimWorldLeague Shafi "
         "TwilightAngle\n";
}

} // namespace

int main(int argc, char **argv) {
  /**
   * Ensure floating-point values print with enough digits to round-trip
   * exactly, matching JS's default number-to-string behavior. Without this,
   * std::cout's default precision (6 significant digits) silently truncates
   * values like coordinates.latitude or the Qibla direction.
   */
  std::cout << std::setprecision(std::numeric_limits<double>::max_digits10);

  if (argc < 3) {
    // Optional, but we can use this to check if we are using the TZ fallback
    DateTime dummy;
    if (dummy.isUsingFallback()) {
      std::cout << "[Note] Using ctime fallback for <chrono> tzdb" << std::endl;
    } else {
      std::cout << "[Note] Using <chrono> tzdb" << std::endl;
    }
    printUsage(argv[0]);
    return 1;
  }

  double latitude = std::stod(argv[1]);
  double longitude = std::stod(argv[2]);

  DateTime date = (argc > 3) ? parseDateArg(argv[3]) : DateTime::now();
  std::string method = (argc > 4) ? argv[4] : "MuslimWorldLeague";
  std::string madhab = (argc > 5) ? argv[5] : "Shafi";
  std::string highLatRule = (argc > 6) ? argv[6] : "MiddleOfTheNight";
  std::string shafaq = (argc > 7) ? argv[7] : "General";
  std::string rounding = (argc > 8) ? argv[8] : "Up";
  std::string polar = (argc > 9) ? argv[9] : "Unresolved";
  /**
   * -----------------------------------------------------------------
   * 1. Coordinates + CalculationParameters
   * -----------------------------------------------------------------
   */
  Coordinates coordinates(latitude, longitude);

  CalculationParameters params = resolveMethod(method);
  params.madhab = resolveMadhab(madhab);
  params.highLatitudeRule = resolveHighLatRule(highLatRule);
  params.shafaq = resolveShafaq(shafaq);
  params.rounding = resolveRounding(rounding);
  params.polarCircleResolution = resolvePolar(polar);

  std::cout << "=== Inputs ===\n";
  std::cout << std::setprecision(8);
  std::cout << "Coordinates:      (" << coordinates.latitude << ", "
            << coordinates.longitude << ")\n";
  std::cout << std::setprecision(std::numeric_limits<double>::max_digits10);
  std::cout << "Date:             " << formatUtc(date) << "\n";
  std::cout << "Method:           " << method << "\n";
  std::cout << "Madhab:           " << MadhabUtils::to_string(params.madhab)
            << "\n";
  std::cout << "Rounding:         " << RoundingUtils::to_string(params.rounding)
            << "\n";
  std::cout << "Shafaq:           " << ShafaqUtils::to_string(params.shafaq)
            << "\n\n";

  /**
   * -----------------------------------------------------------------
   * 2. PrayerTimes
   * -----------------------------------------------------------------
   */
  PrayerTimes prayerTimes(coordinates, date, params);

  std::cout << "=== Prayer Times (UTC) ===\n";
  std::cout << "Fajr:             " << formatUtc(prayerTimes.fajr) << "\n";
  std::cout << "Sunrise:          " << formatUtc(prayerTimes.sunrise) << "\n";
  std::cout << "Dhuhr:            " << formatUtc(prayerTimes.dhuhr) << "\n";
  std::cout << "Asr:              " << formatUtc(prayerTimes.asr) << "\n";
  std::cout << "Maghrib:          " << formatUtc(prayerTimes.maghrib) << "\n";
  std::cout << "Isha:             " << formatUtc(prayerTimes.isha) << "\n\n";

  /**
   * -----------------------------------------------------------------
   * 3. Convenience utilities: timeForPrayer / currentPrayer / nextPrayer
   * -----------------------------------------------------------------
   */
  std::cout << "=== Convenience Utilities ===\n";
  for (Prayer p : {Prayer::Fajr, Prayer::Sunrise, Prayer::Dhuhr, Prayer::Asr,
                   Prayer::Maghrib, Prayer::Isha}) {
    auto t = prayerTimes.timeForPrayer(p);
    std::cout << "timeForPrayer(" << prayerName(p) << "):  "
              << (t.has_value() ? formatUtc(*t) : std::string("(none)"))
              << "\n";
  }

  Prayer current = prayerTimes.currentPrayer();
  Prayer next = prayerTimes.nextPrayer();
  std::cout << "currentPrayer():  " << prayerName(current) << "\n";
  std::cout << "nextPrayer():     " << prayerName(next) << "\n";
  auto nextTime = prayerTimes.timeForPrayer(next);
  if (nextTime.has_value()) {
    std::cout << "Time of next:     " << formatUtc(*nextTime) << "\n";
  }
  std::cout << "\n";

  /**
   * -----------------------------------------------------------------
   * 4. Sunnah Times
   * -----------------------------------------------------------------
   */
  SunnahTimes sunnahTimes(prayerTimes);
  std::cout << "=== Sunnah Times (UTC) ===\n";
  std::cout << "Middle of the night:    "
            << formatUtc(sunnahTimes.middleOfTheNight) << "\n";
  std::cout << "Last third of the night: "
            << formatUtc(sunnahTimes.lastThirdOfTheNight) << "\n\n";

  /**
   * -----------------------------------------------------------------
   * 5. Qibla direction
   * -----------------------------------------------------------------
   */
  double qiblaDirection = qibla(coordinates);
  std::cout << "=== Qibla ===\n";
  std::cout << "Direction from North: " << qiblaDirection << " degrees\n\n";

  /**
   * -----------------------------------------------------------------
   * 6. HighLatitudeRule::recommended — suggests a rule based on latitude
   * -----------------------------------------------------------------
   */
  HighLatitudeRule recommended_rule = recommended(coordinates);
  std::cout << "=== Recommendations ===\n";
  std::cout << "Recommended high latitude rule for this location: ";
  switch (recommended_rule) {
  case HighLatitudeRule::MiddleOfTheNight:
    std::cout << "MiddleOfTheNight";
    break;
  case HighLatitudeRule::SeventhOfTheNight:
    std::cout << "SeventhOfTheNight";
    break;
  case HighLatitudeRule::TwilightAngle:
    std::cout << "TwilightAngle";
    break;
  }
  std::cout << "\n";

  return 0;
}