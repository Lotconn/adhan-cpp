#include "doctest.h"

#include "DateUtils.hpp"
#include "JSDate.hpp"
#include "MathUtils.hpp"
#include "Rounding.hpp"
#include "TimeComponents.hpp"

#include <cmath>
#include <numbers>

using namespace Adhan;

TEST_CASE("converting between degrees and radians") {
  CHECK(radiansToDegrees(std::numbers::pi) == 180);
  CHECK(degreesToRadians(180) == std::numbers::pi);
  CHECK(radiansToDegrees(std::numbers::pi / 2) == 90);
  CHECK(degreesToRadians(90) == std::numbers::pi / 2);
}

TEST_CASE("normalizes the number to be inside the specified scale") {
  CHECK(normalizeToScale(2.0, -5) == -3);
  CHECK(normalizeToScale(-4, -5) == -4);
  CHECK(normalizeToScale(-6, -5) == -1);

  CHECK(normalizeToScale(-1, 24) == 23);
  CHECK(normalizeToScale(1, 24) == 1);
  CHECK(normalizeToScale(49.0, 24) == 1);

  CHECK(normalizeToScale(361, 360) == 1);
  CHECK(normalizeToScale(360, 360) == 0);
  CHECK(normalizeToScale(259, 360) == 259);
  CHECK(normalizeToScale(2592.0, 360) == 72);

  CHECK(unwindAngle(-45) == 315);
  CHECK(unwindAngle(361.0) == 1);
  CHECK(unwindAngle(360) == 0);
  CHECK(unwindAngle(259.0) == 259);
  CHECK(unwindAngle(2592) == 72);

  CHECK(std::abs(normalizeToScale(360.1, 360) - 0.1) < std::pow(10, -9) / 2.0);
}

TEST_CASE(
    "find the same angle constrained within the first and fourth quadrants") {
  CHECK(quadrantShiftAngle(360.0) == 0);
  CHECK(quadrantShiftAngle(361.0) == 1);
  CHECK(quadrantShiftAngle(1.0) == 1);
  CHECK(quadrantShiftAngle(-1.0) == -1);
  CHECK(quadrantShiftAngle(-181.0) == 179);
  CHECK(quadrantShiftAngle(180.0) == 180);
  CHECK(quadrantShiftAngle(359.0) == -1);
  CHECK(quadrantShiftAngle(-359.0) == 1);
  CHECK(quadrantShiftAngle(1261.0) == -179);
  CHECK(std::abs(quadrantShiftAngle(-360.1) - (-0.1)) < std::pow(10, -9) / 2.0);
}

TEST_CASE(
    "break down an hour value into separate hour, minute, and second values") {
  TimeComponents comps1(15.199);
  CHECK(comps1.hours == 15);
  CHECK(comps1.minutes == 11);
  CHECK(comps1.seconds == 56);

  TimeComponents comps2(1.0084);
  CHECK(comps2.hours == 1);
  CHECK(comps2.minutes == 0);
  CHECK(comps2.seconds == 30);

  TimeComponents comps3(1.0083);
  CHECK(comps3.hours == 1);
  CHECK(comps3.minutes == 0);

  TimeComponents comps4(2.1);
  CHECK(comps4.hours == 2);
  CHECK(comps4.minutes == 6);

  TimeComponents comps5(3.5);
  CHECK(comps5.hours == 3);
  CHECK(comps5.minutes == 30);
}

TEST_CASE("rounding a date to the closest minute") {
  JSDate date1 = roundedMinute(JSDate(2015, 0, 1, 10, 2, 29));
  CHECK(date1.getMinutes() == 2);
  CHECK(date1.getSeconds() == 0);

  JSDate date2 = roundedMinute(JSDate(2015, 0, 1, 10, 2, 31));
  CHECK(date2.getMinutes() == 3);
  CHECK(date2.getSeconds() == 0);

  JSDate date3 = roundedMinute(JSDate(2015, 0, 1, 10, 2, 29), Rounding::Up);
  CHECK(date3.getMinutes() == 3);
  CHECK(date3.getSeconds() == 0);

  JSDate date4 = roundedMinute(JSDate(2015, 0, 1, 10, 2, 29), Rounding::None);
  CHECK(date4.getMinutes() == 2);
  CHECK(date4.getSeconds() == 29);

  JSDate date5 =
      roundedMinute(JSDate(2015, 0, 1, 10, 2, 29), Rounding::Nearest);
  CHECK(date5.getMinutes() == 2);
  CHECK(date5.getSeconds() == 0);
}

TEST_CASE("adding days to date") {
  JSDate date1(2015, 10, 1, 0, 0, 0);
  CHECK(date1.getDate() == 1);

  JSDate date2 = dateByAddingDays(date1, 1);
  CHECK(date2.getDate() == 2);
}

TEST_CASE("determine if a year is a leap year") {
  CHECK_FALSE(isLeapYear(2015));
  CHECK(isLeapYear(2016));
  CHECK(isLeapYear(1600));
  CHECK(isLeapYear(2000));
  CHECK(isLeapYear(2400));
  CHECK_FALSE(isLeapYear(1700));
  CHECK_FALSE(isLeapYear(1800));
  CHECK_FALSE(isLeapYear(1900));
  CHECK_FALSE(isLeapYear(2100));
  CHECK_FALSE(isLeapYear(2200));
  CHECK_FALSE(isLeapYear(2300));
  CHECK_FALSE(isLeapYear(2500));
  CHECK_FALSE(isLeapYear(2600));
}