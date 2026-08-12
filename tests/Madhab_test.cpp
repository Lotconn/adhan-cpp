#include "doctest.h"

#include <adhan/Madhab.hpp>

using namespace Adhan;

TEST_CASE("getting the madhab shadow length") {
  CHECK(shadow_length(Madhab::Shafi) == 1);
  CHECK(shadow_length(Madhab::Hanafi) == 2);
  CHECK_THROWS(shadow_length(static_cast<Madhab>(999)));
}
