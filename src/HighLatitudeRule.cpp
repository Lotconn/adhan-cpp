#include "HighLatitudeRule.hpp"

HighLatitudeRule recommended(const Coordinates &coordinates) {
  if (coordinates.latitude > 48) {
    return HighLatitudeRule::SeventhOfTheNight;
  } else {
    return HighLatitudeRule::MiddleOfTheNight;
  }
}