#include "TimeComponents.hpp"
#include <chrono>
#include <cmath>

TimeComponents::TimeComponents(double num) {
  this->hours = static_cast<int>(std::floor(num));
  this->minutes = static_cast<int>(std::floor(num - hours) * 60);
  this->seconds =
      static_cast<int>(std::floor((num - (hours + minutes / 60.0)) * 60 * 60));
}

auto TimeComponents::utcDate(int year, int month, int date) {
  return std::chrono::sys_days{std::chrono::year{year} / (month + 1) / date} +
         +std::chrono::hours{hours} + std::chrono::minutes{minutes} +
         std::chrono::seconds{seconds};
}