/**
 * Adhan C++ CLI Demo
 *
 * Exercises the full public API of the adhan library: prayer time
 * calculation, Sunnah times, Qibla direction, and the enum helpers.
 * Modeled on the usage patterns shown in the adhan-js README.
 *
 * Everything here is UTC. The library takes a plain calendar date and
 * hands back UTC instants, so it never touches a time zone database.
 * Working out which day a user is currently in, or printing a local wall
 * clock time, is left to whoever embeds the library. Keeping this demo on
 * UTC means it builds and runs the same way on every platform.
 *
 * Usage:
 *   adhan-cli --latitude <deg> --longitude <deg> [options]
 *
 * Example:
 *   adhan-cli -a 35.78056 -o -78.6389 -d 2026-01-01 -m NorthAmerica -M Hanafi
 */

#include <adhan/CalculationMethod.hpp>
#include <adhan/CalculationParameters.hpp>
#include <adhan/Coordinates.hpp>
#include <adhan/DateUtils.hpp>
#include <adhan/HighLatitudeRule.hpp>
#include <adhan/Madhab.hpp>
#include <adhan/PolarCircleResolution.hpp>
#include <adhan/Prayer.hpp>
#include <adhan/PrayerTimes.hpp>
#include <adhan/Qibla.hpp>
#include <adhan/Rounding.hpp>
#include <adhan/Shafaq.hpp>
#include <adhan/SunnahTimes.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

using namespace Adhan;

namespace {

/**
 * @brief Renders an instant as "YYYY-MM-DD HH:MM:SS UTC".
 *
 * An absent time means the sun never reached the needed angle, which
 * happens near the poles unless a resolution strategy is picked.
 */
std::string formatUtc(const OptInstant &time) {
  if (!time) {
    return "(none: polar location or date, try --polar-circle-resolution)";
  }

  const auto day = std::chrono::floor<std::chrono::days>(*time);
  const auto timeOfDay = std::chrono::floor<std::chrono::seconds>(*time - day);

  std::ostringstream oss;
  oss << std::chrono::year_month_day{day} << " "
      << std::chrono::hh_mm_ss{timeOfDay} << " UTC";
  return oss.str();
}

/**
 * @brief Lowercases a string so option values can be typed in any casing.
 */
std::string toLower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return s;
}

/**
 * Option values are matched against these tables rather than the library's
 * own from_string helpers. Those helpers disagree with each other on
 * casing, and the one for Rounding returns Rounding::None for anything it
 * does not recognise instead of complaining, so a typo would pass silently.
 */
using MethodFactory = CalculationParameters (*)();

constexpr std::array<std::pair<std::string_view, MethodFactory>, 13> METHODS{{
    {"muslimworldleague", CalculationMethod::MuslimWorldLeague},
    {"egyptian", CalculationMethod::Egyptian},
    {"karachi", CalculationMethod::Karachi},
    {"ummalqura", CalculationMethod::UmmAlQura},
    {"dubai", CalculationMethod::Dubai},
    {"moonsightingcommittee", CalculationMethod::MoonsightingCommittee},
    {"northamerica", CalculationMethod::NorthAmerica},
    {"kuwait", CalculationMethod::Kuwait},
    {"qatar", CalculationMethod::Qatar},
    {"singapore", CalculationMethod::Singapore},
    {"turkey", CalculationMethod::Turkey},
    {"tehran", CalculationMethod::Tehran},
    {"other", CalculationMethod::Other},
}};

constexpr std::array<std::pair<std::string_view, Madhab>, 2> MADHABS{{
    {"shafi", Madhab::Shafi},
    {"hanafi", Madhab::Hanafi},
}};

constexpr std::array<std::pair<std::string_view, HighLatitudeRule>, 3>
    HIGH_LATITUDE_RULES{{
        {"middleofthenight", HighLatitudeRule::MiddleOfTheNight},
        {"seventhofthenight", HighLatitudeRule::SeventhOfTheNight},
        {"twilightangle", HighLatitudeRule::TwilightAngle},
    }};

constexpr std::array<std::pair<std::string_view, Shafaq>, 3> SHAFAQS{{
    {"general", Shafaq::General},
    {"ahmer", Shafaq::Ahmer},
    {"abyad", Shafaq::Abyad},
}};

constexpr std::array<std::pair<std::string_view, Rounding>, 3> ROUNDINGS{{
    {"nearest", Rounding::Nearest},
    {"up", Rounding::Up},
    {"none", Rounding::None},
}};

constexpr std::array<std::pair<std::string_view, PolarCircleResolution>, 3>
    POLAR_RESOLUTIONS{{
        {"unresolved", PolarCircleResolution::Unresolved},
        {"aqrabbalad", PolarCircleResolution::AqrabBalad},
        {"aqrabyaum", PolarCircleResolution::AqrabYaum},
    }};

/**
 * @brief Looks a value up in one of the tables above.
 * @throws std::invalid_argument if the key is not present.
 */
template <typename Table>
auto lookup(const Table &table, const std::string &key, const char *what) {
  const auto folded = toLower(key);
  const auto it =
      std::find_if(table.begin(), table.end(), [&](const auto &entry) {
        return entry.first == folded;
      });

  if (it == table.end()) {
    throw std::invalid_argument("Unknown " + std::string(what) + ": " + key);
  }
  return it->second;
}

/**
 * @brief Reads a "YYYY-MM-DD" argument.
 *
 * The result is not checked for being a real date here. main() does that
 * with year_month_day::ok(), which catches things like 2026-02-30.
 */
std::chrono::year_month_day parseDate(const std::string &s) {
  if (s.size() != 10 || s[4] != '-' || s[7] != '-') {
    throw std::invalid_argument("Date must look like YYYY-MM-DD, got: " + s);
  }

  return std::chrono::year_month_day{
      std::chrono::year{std::stoi(s.substr(0, 4))},
      std::chrono::month{static_cast<unsigned>(std::stoi(s.substr(5, 2)))},
      std::chrono::day{static_cast<unsigned>(std::stoi(s.substr(8, 2)))}};
}

/**
 * @brief Today's date in UTC.
 *
 * A caller who wants the user's local day instead would convert the
 * current instant through std::chrono::zoned_time first. That needs a
 * time zone database, which is why this demo does not do it.
 */
std::chrono::year_month_day today() {
  return std::chrono::year_month_day{
      std::chrono::floor<std::chrono::days>(now())};
}

void printLibInfo() {
#ifdef USING_SHARED_ADHAN_LIB
  std::cout << "[Note] Using shared adhan library\n";
#else
  std::cout << "[Note] Using static adhan library\n";
#endif
}

void printUsage(const char *progName) {
  std::cout << "Adhan C++ CLI\n\nUsage:\n  " << progName << " [OPTIONS]\n\n"
            << R"(Values are case insensitive.

Required:
  -a, --latitude <degrees>      Latitude in decimal degrees.
  -o, --longitude <degrees>     Longitude in decimal degrees.

Options:
  -d, --date <YYYY-MM-DD>       Day to calculate, read as a UTC day.
                                Default: today in UTC.

  -m, --method <method>         Default: MuslimWorldLeague
                                {MuslimWorldLeague | Egyptian | Karachi |
                                 UmmAlQura | Dubai | MoonsightingCommittee |
                                 NorthAmerica | Kuwait | Qatar | Singapore |
                                 Turkey | Tehran | Other}

  -M, --madhab <madhab>         Default: Shafi
                                {Shafi | Hanafi}

  -H, --high-latitude-rule <r>  Default: MiddleOfTheNight
                                {MiddleOfTheNight | SeventhOfTheNight |
                                 TwilightAngle}

  -s, --shafaq <shafaq>         Default: General
                                {General | Ahmer | Abyad}

  -r, --rounding <rounding>     Default: Nearest
                                {Nearest | Up | None}

  -p, --polar-circle-resolution <resolution>
                                Default: Unresolved
                                {Unresolved | AqrabBalad | AqrabYaum}

  -h, --help                    Show this message.

Example:
  )" << progName
            << " -a 23.775787 -o 90.368047 -d 2026-07-24 -m MuslimWorldLeague"
               " -M Shafi -H TwilightAngle -r Nearest\n";
}

} // namespace

int main(int argc, char **argv) { // NOLINT
  /**
   * Print enough digits that doubles round-trip exactly, matching what JS
   * does by default. Without this, std::cout's 6 significant digits would
   * quietly truncate the coordinates and the Qibla direction.
   */
  std::cout << std::setprecision(std::numeric_limits<double>::max_digits10);

  if (argc < 3) {
    printLibInfo();
    printUsage(argv[0]);
    return 1;
  }

  std::optional<double> latitude;
  std::optional<double> longitude;

  auto date = today();
  CalculationParameters params = CalculationMethod::MuslimWorldLeague();

  try {
    /**
     * Reads the value that follows the flag at position i, moving i past
     * it so the loop lands on the next flag.
     */
    auto value = [&](int &i) -> std::string {
      if (i + 1 >= argc) {
        throw std::invalid_argument(
            std::string("Missing value for ") + argv[i]);
      }
      return argv[++i];
    };

    for (int i = 1; i < argc; ++i) {
      const std::string arg = argv[i];

      if (arg == "--help" || arg == "-h") {
        printUsage(argv[0]);
        return 0;
      }
      if (arg == "--latitude" || arg == "-a") {
        latitude = std::stod(value(i));
      } else if (arg == "--longitude" || arg == "-o") {
        longitude = std::stod(value(i));
      } else if (arg == "--date" || arg == "-d") {
        date = parseDate(value(i));
      } else if (arg == "--method" || arg == "-m") {
        params = lookup(METHODS, value(i), "method")();
      } else if (arg == "--madhab" || arg == "-M") {
        params.madhab = lookup(MADHABS, value(i), "madhab");
      } else if (arg == "--high-latitude-rule" || arg == "-H") {
        params.highLatitudeRule =
            lookup(HIGH_LATITUDE_RULES, value(i), "high latitude rule");
      } else if (arg == "--shafaq" || arg == "-s") {
        params.shafaq = lookup(SHAFAQS, value(i), "shafaq");
      } else if (arg == "--rounding" || arg == "-r") {
        params.rounding = lookup(ROUNDINGS, value(i), "rounding");
      } else if (arg == "--polar-circle-resolution" || arg == "-p") {
        params.polarCircleResolution =
            lookup(POLAR_RESOLUTIONS, value(i), "polar circle resolution");
      } else {
        throw std::invalid_argument("Unknown option: " + arg);
      }
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << "\n\n";
    printUsage(argv[0]);
    return 1;
  }

  if (!latitude || !longitude) {
    std::cerr << "Both `--latitude` and `--longitude` are required.\n\n";
    printLibInfo();
    printUsage(argv[0]);
    return 1;
  }

  /**
   * year_month_day accepts anything you build it from, so a typo like
   * 2026-02-30 only shows up on this check.
   */
  if (!date.ok()) {
    std::cerr << "Not a real date: " << date << "\n";
    return 1;
  }

  /**
   * -----------------------------------------------------------------
   * 1. Coordinates + CalculationParameters
   * -----------------------------------------------------------------
   */
  const Coordinates coordinates(latitude.value(), longitude.value());

  std::cout << "=== Inputs ===\n";
  std::cout << std::setprecision(8);
  std::cout << "Coordinates:      (" << coordinates.latitude << ", "
            << coordinates.longitude << ")\n";
  std::cout << std::setprecision(std::numeric_limits<double>::max_digits10);
  std::cout << "Date (UTC day):   " << date << "\n";
  std::cout << "Method:           " << params.method << "\n";
  std::cout << "Madhab:           " << MadhabUtils::to_string(params.madhab)
            << "\n";
  std::cout << "High latitude:    "
            << HighLatitudeRuleUtils::to_string(params.highLatitudeRule)
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
  const PrayerTimes prayerTimes(coordinates, date, params);

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
  std::cout << std::left;
  for (const Prayer p : {Prayer::Fajr, Prayer::Sunrise, Prayer::Dhuhr,
                         Prayer::Asr, Prayer::Maghrib, Prayer::Isha}) {
    const std::string label =
        "timeForPrayer(" + std::string(PrayerUtils::to_string(p)) + "):";
    std::cout << std::setw(24) << label
              << formatUtc(prayerTimes.timeForPrayer(p)) << "\n";
  }
  std::cout << std::right;

  /**
   * currentPrayer and nextPrayer default to the current instant, so the
   * answers only mean something when the requested day is today.
   */
  const Prayer next = prayerTimes.nextPrayer();
  std::cout << "currentPrayer():  "
            << PrayerUtils::to_string(prayerTimes.currentPrayer()) << "\n";
  std::cout << "nextPrayer():     " << PrayerUtils::to_string(next) << "\n";

  /**
   * Prayer::None means Isha has already gone, so there is no time to look
   * up. Asking anyway would report it as missing, which reads like a polar
   * failure rather than the end of the day.
   */
  if (next != Prayer::None) {
    std::cout << "Time of next:     "
              << formatUtc(prayerTimes.timeForPrayer(next)) << "\n";
  }
  std::cout << "\n";

  /**
   * -----------------------------------------------------------------
   * 4. Sunnah Times
   * -----------------------------------------------------------------
   */
  const SunnahTimes sunnahTimes(prayerTimes);
  std::cout << "=== Sunnah Times (UTC) ===\n";
  std::cout << "Middle of the night:     "
            << formatUtc(sunnahTimes.middleOfTheNight) << "\n";
  std::cout << "Last third of the night: "
            << formatUtc(sunnahTimes.lastThirdOfTheNight) << "\n\n";

  /**
   * -----------------------------------------------------------------
   * 5. Qibla direction
   * -----------------------------------------------------------------
   */
  std::cout << "=== Qibla ===\n";
  std::cout << "Direction from North: " << qibla(coordinates) << " degrees\n\n";

  /**
   * -----------------------------------------------------------------
   * 6. recommended() suggests a high latitude rule based on latitude
   * -----------------------------------------------------------------
   */
  std::cout << "=== Recommendations ===\n";
  std::cout << "Recommended high latitude rule for this location: "
            << HighLatitudeRuleUtils::to_string(recommended(coordinates))
            << "\n";

  return 0;
}
