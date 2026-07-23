#include <CalculationParameters.hpp>
#include <stdexcept>

CalculationParameters::CalculationParameters(std::optional<std::string> method,
                                             double fajrAngle, double ishaAngle,
                                             double ishaInterval,
                                             double maghribAngle)
    : fajrAngle(fajrAngle), ishaAngle(ishaAngle), ishaInterval(ishaInterval),
      maghribAngle(maghribAngle) {

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
    throw std::runtime_error("Invalid high latitude rule found when attempting "
                             "to compute night portions");
  }
}