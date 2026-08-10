#include "doctest.h"

#include <adhan/Coordinates.hpp>
#include <adhan/HighLatitudeRule.hpp>

using namespace Adhan;

TEST_CASE("getting recommended high latitude rule") {
  Coordinates coords1(45.983226, -3.216649);
  CHECK(recommended(coords1) == HighLatitudeRule::MiddleOfTheNight);

  Coordinates coords2(48.983226, -3.216649);
  CHECK(recommended(coords2) == HighLatitudeRule::SeventhOfTheNight);
}
