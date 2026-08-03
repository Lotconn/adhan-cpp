#include "doctest.h"

#include <adhan/CalculationMethod.hpp>
#include <adhan/CalculationParameters.hpp>
#include <adhan/HighLatitudeRule.hpp>

using namespace Adhan;

TEST_CASE("Verifying the night portion defined by the high latitude rule") {
  CalculationParameters p1(std::nullopt, 18, 18);
  p1.highLatitudeRule = HighLatitudeRule::MiddleOfTheNight;
  CHECK(p1.nightPortions().fajr == doctest::Approx(0.5));
  CHECK(p1.nightPortions().isha == doctest::Approx(0.5));

  CalculationParameters p2(std::nullopt, 18, 18);
  p2.highLatitudeRule = HighLatitudeRule::SeventhOfTheNight;
  CHECK(p2.nightPortions().fajr == doctest::Approx(1.0 / 7));
  CHECK(p2.nightPortions().isha == doctest::Approx(1.0 / 7));

  CalculationParameters p3(std::nullopt, 10, 15);
  p3.highLatitudeRule = HighLatitudeRule::TwilightAngle;
  CHECK(p3.nightPortions().fajr == doctest::Approx(10.0 / 60));
  CHECK(p3.nightPortions().isha == doctest::Approx(15.0 / 60));

  /**
   * The TS test casts an invalid/"fake" enum value to exercise the throw
   * path in the default case of nightPortions()'s switch. C++'s enum class
   * has no equivalent unchecked cast from a string, so we reproduce the same
   * "invalid enum value" condition directly via static_cast. It would be ok
   * to get rid of this check, but I kept it to (somewhat) maintain parity
   */
  CalculationParameters p4(std::nullopt, 10, 15);
  p4.highLatitudeRule = static_cast<HighLatitudeRule>(999);

  CHECK_THROWS(p4.nightPortions());
}

TEST_CASE("Verifying the angles defined by the calculation method") {
  CalculationParameters p1 = CalculationMethod::MuslimWorldLeague();
  CHECK(p1.fajrAngle == doctest::Approx(18));
  CHECK(p1.ishaAngle == doctest::Approx(17));
  CHECK(p1.ishaInterval == doctest::Approx(0));
  CHECK(p1.method == "MuslimWorldLeague");

  CalculationParameters p2 = CalculationMethod::Egyptian();
  CHECK(p2.fajrAngle == doctest::Approx(19.5));
  CHECK(p2.ishaAngle == doctest::Approx(17.5));
  CHECK(p2.ishaInterval == doctest::Approx(0));
  CHECK(p2.method == "Egyptian");

  CalculationParameters p3 = CalculationMethod::Karachi();
  CHECK(p3.fajrAngle == doctest::Approx(18));
  CHECK(p3.ishaAngle == doctest::Approx(18));
  CHECK(p3.ishaInterval == doctest::Approx(0));
  CHECK(p3.method == "Karachi");

  CalculationParameters p4 = CalculationMethod::UmmAlQura();
  CHECK(p4.fajrAngle == doctest::Approx(18.5));
  CHECK(p4.ishaAngle == doctest::Approx(0));
  CHECK(p4.ishaInterval == doctest::Approx(90));
  CHECK(p4.method == "UmmAlQura");

  CalculationParameters p5 = CalculationMethod::Dubai();
  CHECK(p5.fajrAngle == doctest::Approx(18.2));
  CHECK(p5.ishaAngle == doctest::Approx(18.2));
  CHECK(p5.ishaInterval == doctest::Approx(0));
  CHECK(p5.method == "Dubai");

  CalculationParameters p6 = CalculationMethod::MoonsightingCommittee();
  CHECK(p6.fajrAngle == doctest::Approx(18));
  CHECK(p6.ishaAngle == doctest::Approx(18));
  CHECK(p6.ishaInterval == doctest::Approx(0));
  CHECK(p6.method == "MoonsightingCommittee");

  CalculationParameters p7 = CalculationMethod::NorthAmerica();
  CHECK(p7.fajrAngle == doctest::Approx(15));
  CHECK(p7.ishaAngle == doctest::Approx(15));
  CHECK(p7.ishaInterval == doctest::Approx(0));
  CHECK(p7.method == "NorthAmerica");

  CalculationParameters p8 = CalculationMethod::Other();
  CHECK(p8.fajrAngle == doctest::Approx(0));
  CHECK(p8.ishaAngle == doctest::Approx(0));
  CHECK(p8.ishaInterval == doctest::Approx(0));
  CHECK(p8.method == "Other");

  CalculationParameters p9 = CalculationMethod::Kuwait();
  CHECK(p9.fajrAngle == doctest::Approx(18));
  CHECK(p9.ishaAngle == doctest::Approx(17.5));
  CHECK(p9.ishaInterval == doctest::Approx(0));
  CHECK(p9.method == "Kuwait");

  CalculationParameters p10 = CalculationMethod::Qatar();
  CHECK(p10.fajrAngle == doctest::Approx(18));
  CHECK(p10.ishaAngle == doctest::Approx(0));
  CHECK(p10.ishaInterval == doctest::Approx(90));
  CHECK(p10.method == "Qatar");

  CalculationParameters p11 = CalculationMethod::Singapore();
  CHECK(p11.fajrAngle == doctest::Approx(20));
  CHECK(p11.ishaAngle == doctest::Approx(18));
  CHECK(p11.ishaInterval == doctest::Approx(0));
  CHECK(p11.method == "Singapore");

  CalculationParameters p12 = CalculationMethod::Tehran();
  CHECK(p12.fajrAngle == doctest::Approx(17.7));
  CHECK(p12.ishaAngle == doctest::Approx(14));
  CHECK(p12.ishaInterval == doctest::Approx(0));
  CHECK(p12.method == "Tehran");

  CalculationParameters p13 = CalculationMethod::Turkey();
  CHECK(p13.fajrAngle == doctest::Approx(18));
  CHECK(p13.ishaAngle == doctest::Approx(17));
  CHECK(p13.ishaInterval == doctest::Approx(0));
  CHECK(p13.method == "Turkey");

  CalculationParameters p14(std::nullopt, 18, 17);
  CHECK(p14.fajrAngle == doctest::Approx(18));
  CHECK(p14.ishaAngle == doctest::Approx(17));
  CHECK(p14.ishaInterval == doctest::Approx(0));
  CHECK(p14.method == "Other");
}