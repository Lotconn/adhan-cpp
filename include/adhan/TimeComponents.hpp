#ifndef TIMECOMPONENTS_HPP
#define TIMECOMPONENTS_HPP

#include "DateTime.hpp"

namespace Adhan {

class TimeComponents {
public:
  // NOLINTBEGIN(cppcoreguidelines-non-private-member-variables-in-classes)
  int hours;
  int minutes;
  int seconds;
  // NOLINTEND(cppcoreguidelines-non-private-member-variables-in-classes)

  explicit TimeComponents(double num);

  DateTime utcDate(int year, int month, int date) const;
  bool isValid() const {
    return valid_;
  } // <-- added

private:
  bool valid_;
};
} // namespace Adhan

#endif // TIMECOMPONENTS_HPP