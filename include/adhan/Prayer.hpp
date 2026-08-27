#ifndef PRAYER_HPP
#define PRAYER_HPP

#include <cstdint>
#include <string_view>

namespace Adhan {

enum class Prayer : std::int8_t {
  Fajr,
  Sunrise,
  Dhuhr,
  Asr,
  Maghrib,
  Isha,
  None,
};

namespace PrayerUtils {

constexpr std::string_view to_string(Prayer prayer) {
  switch (prayer) {
  case Prayer::Fajr:
    return "Fajr";
  case Prayer::Sunrise:
    return "Sunrise";
  case Prayer::Dhuhr:
    return "Dhuhr";
  case Prayer::Asr:
    return "Asr";
  case Prayer::Maghrib:
    return "Maghrib";
  case Prayer::Isha:
    return "Isha";
  case Prayer::None:
  default:
    return "None";
  }
}

constexpr Prayer from_string(std::string_view value) {
  if (value == "Fajr") {
    return Prayer::Fajr;
  }
  if (value == "Sunrise") {
    return Prayer::Sunrise;
  }
  if (value == "Dhuhr") {
    return Prayer::Dhuhr;
  }
  if (value == "Asr") {
    return Prayer::Asr;
  }
  if (value == "Maghrib") {
    return Prayer::Maghrib;
  }
  if (value == "Isha") {
    return Prayer::Isha;
  }

  /* Defaults to "none" */
  return Prayer::None;
}

} // namespace PrayerUtils
} // namespace Adhan

#endif /* PRAYER_HPP */