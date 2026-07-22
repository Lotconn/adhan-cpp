#ifndef HIGHLATITUDERULE_HPP
#define HIGHLATITUDERULE_HPP

#include "Coordinates.hpp"
#include <string>

namespace HighLatitudeRule {
inline const std::string MiddleOfTheNight = "middleofthenight";
inline const std::string SeventhOfTheNight = "seventhofthenight";
inline const std::string TwilightAngle = "twilightangle";

std::string recommended(const Coordinates &coordinates);

} // namespace HighLatitudeRule

#endif /* HIGHLATITUDERULE_HPP */