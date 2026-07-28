#ifndef HIGHLATITUDERULE_HPP
#define HIGHLATITUDERULE_HPP

#include "Coordinates.hpp"
#include <stdexcept>
#include <string_view>

namespace Adhan {

enum class HighLatitudeRule {
  MiddleOfTheNight,
  SeventhOfTheNight,
  TwilightAngle,
};

namespace HighLatitudeRuleUtils {
/**
 * @brief Given a HighLatitudeRule (e.g.: HighLatitudeRule::MiddleOfTheNight),
 * returns a std::string_view
 *
 * @param r
 * @return constexpr std::string_view
 */
constexpr std::string_view to_string(HighLatitudeRule h) {
  switch (h) {

  case HighLatitudeRule::MiddleOfTheNight:
    return "middleofthenight";
  case HighLatitudeRule::SeventhOfTheNight:
    return "seventhofthenight";
  case HighLatitudeRule::TwilightAngle:
    return "twilightangle";
  }

  throw std::logic_error("Invalid high latitude rule");
  return {};
}

/**
 * @brief Given a string (e.g.: "middleofthenight"), returns a
 * `HighLatitudeRule`
 *
 * @param s
 * @return constexpr HighLatitudeRule
 */
constexpr HighLatitudeRule from_string(std::string_view s) {
  if (s == "middleofthenight")
    return HighLatitudeRule::MiddleOfTheNight;
  if (s == "seventhofthenight")
    return HighLatitudeRule::SeventhOfTheNight;
  if (s == "twilightangle")
    return HighLatitudeRule::TwilightAngle;

  throw std::logic_error("Invalid high latitude rule");
  return {};
}
} // namespace HighLatitudeRuleUtils

HighLatitudeRule recommended(const Coordinates &coordinates);
} // namespace Adhan

#endif /* HIGHLATITUDERULE_HPP */