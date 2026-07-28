#ifndef TIMECOMPONENTS_HPP
#define TIMECOMPONENTS_HPP

#include "JSDate.hpp"

namespace Adhan {

class TimeComponents {
public:
  int hours;
  int minutes;
  int seconds;

  explicit TimeComponents(double num);

  JSDate utcDate(int year, int month, int date) const;
  bool isValid() const { return valid_; } // <-- added

private:
  bool valid_;
};
} // namespace Adhan

#endif // TIMECOMPONENTS_HPP