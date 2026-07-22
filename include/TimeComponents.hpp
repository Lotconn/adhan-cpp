#ifndef TIMECOMPONENTS_HPP
#define TIMECOMPONENTS_HPP

#include "JSDate.hpp"

class TimeComponents {
public:
  int hours;
  int minutes;
  int seconds;

  explicit TimeComponents(double num);

  JSDate utcDate(int year, int month, int date) const;

private:
  bool valid_;
};

#endif // TIMECOMPONENTS_HPP