#include <adhan/TimeComponents.hpp>
#include <chrono>
#include <cmath>

namespace Adhan {

TimeComponents::TimeComponents(double num) {
  valid_ = !std::isnan(num);
  if (!valid_) {
    hours = 0;
    minutes = 0;
    seconds = 0;
    return;
  }

  hours = static_cast<int>(std::floor(num));
  minutes = static_cast<int>(std::floor((num - hours) * 60));
  seconds =
      static_cast<int>(std::floor((num - (hours + minutes / 60.0)) * 60 * 60));
}

DateTime TimeComponents::utcDate(int year, int month, int date) const {
  if (!valid_) {
    return DateTime::invalid();
  }

  using namespace std::chrono;
  auto tp =
      std::chrono::sys_days{std::chrono::year{year} / (month + 1) / date} +
      std::chrono::hours{hours} + std::chrono::minutes{minutes} +
      std::chrono::seconds{seconds};
  return DateTime(tp);
}
} // namespace Adhan