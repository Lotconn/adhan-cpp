#ifndef PRAYER_HPP
#define PRAYER_HPP

#include <optional>
#include <string_view>

enum class Prayer {
  Fajr,
  Sunrise,
  Dhuhr,
  Asr,
  Maghrib,
  Isha,
  None,
};

constexpr std::string_view to_string(Prayer prayer);

std::optional<Prayer> from_string(std::string_view value);

#endif /* PRAYER_HPP */