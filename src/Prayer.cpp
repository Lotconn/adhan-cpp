#include "Prayer.hpp"

#include <cassert>

constexpr std::string_view to_string(Prayer prayer) {
  switch (prayer) {
  case Prayer::Fajr:
    return "fajr";
  case Prayer::Sunrise:
    return "sunrise";
  case Prayer::Dhuhr:
    return "dhuhr";
  case Prayer::Asr:
    return "asr";
  case Prayer::Maghrib:
    return "maghrib";
  case Prayer::Isha:
    return "isha";
  case Prayer::None:
    return "none";
  }

  assert(false && "Invalid Prayer");
  return {};
}

std::optional<Prayer> from_string(std::string_view value) {
  if (value == "fajr")
    return Prayer::Fajr;
  if (value == "sunrise")
    return Prayer::Sunrise;
  if (value == "dhuhr")
    return Prayer::Dhuhr;
  if (value == "asr")
    return Prayer::Asr;
  if (value == "maghrib")
    return Prayer::Maghrib;
  if (value == "isha")
    return Prayer::Isha;
  if (value == "none")
    return Prayer::None;

  return std::nullopt;
}