#include <adhan/CalculationParameters.hpp>
#include <stdexcept>

namespace Adhan {

CalculationParameters::CalculationParameters(
    std::optional<std::string> method, double fajrAngle, double ishaAngle,
    double ishaInterval, double maghribAngle)
    : fajrAngle(fajrAngle), ishaAngle(ishaAngle), ishaInterval(ishaInterval),
      maghribAngle(maghribAngle) {

  /* check valid range */
  if (fajrAngle < 0.0 || fajrAngle > 90.0) {
    throw std::invalid_argument(
        "CalculationParameters: fajrAngle must be in [0, 90]");
  }
  if (ishaAngle < 0.0 || ishaAngle > 90.0) {
    throw std::invalid_argument(
        "CalculationParameters: ishaAngle must be in [0, 90]");
  }
  if (ishaInterval < 0.0) {
    throw std::invalid_argument(
        "CalculationParameters: ishaInterval must be >= 0");
  }
  if (maghribAngle < 0.0 || maghribAngle > 90.0) {
    throw std::invalid_argument(
        "CalculationParameters: maghribAngle must be in [0, 90]");
  }

  /* we don't want a breaking change */
  this->method = method.value_or("Other");
}

NightPortions CalculationParameters::nightPortions() const {
  switch (highLatitudeRule) {
  case HighLatitudeRule::MiddleOfTheNight:
    return {1.0 / 2, 1.0 / 2};
  case HighLatitudeRule::SeventhOfTheNight:
    return {1.0 / 7, 1.0 / 7};
  case HighLatitudeRule::TwilightAngle:
    return {fajrAngle / 60, ishaAngle / 60};
  default:
    throw std::runtime_error(
        "Invalid high latitude rule found when attempting "
        "to compute night portions");
  }
}
} // namespace Adhan