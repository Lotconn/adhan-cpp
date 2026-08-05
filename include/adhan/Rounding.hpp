#ifndef ROUNDING_HPP
#define ROUNDING_HPP

#include <cstdint>
#include <stdexcept>
#include <string_view>

namespace Adhan {

enum class Rounding : std::int8_t {
  Nearest,
  Up,
  None,
};

namespace RoundingUtils {

/**
 * @brief Given a Rounding (e.g.: Rounding::Nearest), returns a std::string_view
 *
 * @param r
 * @return constexpr std::string_view
 */
constexpr std::string_view to_string(Rounding r) {
  switch (r) {
  case Rounding::Nearest:
    return "nearest";
  case Rounding::Up:
    return "up";
  case Rounding::None:
    return "none";
  }
  throw std::logic_error("Invalid rounding");
}

/**
 * @brief Given a string (e.g.: "nearest"), returns a `Rounding`
 * If no match is found, returns `Rounding::None` by default
 *
 * @param s
 * @return constexpr Rounding
 */
constexpr Rounding from_string(std::string_view s) {
  if (s == "nearest") {
    return Rounding::Nearest;
  }
  if (s == "up") {
    return Rounding::Up;
  }

  return Rounding::None;
}

} // namespace RoundingUtils
} // namespace Adhan

#endif /* ROUNDING_HPP */