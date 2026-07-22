#ifndef TIMECOMPONENTS_HPP
#define TIMECOMPONENTS_HPP

class TimeComponents {
  int hours;
  int minutes;
  int seconds;

public:
  TimeComponents(double num);
  auto utcDate(int year, int month, int date);
};

#endif /* TIMECOMPONENTS_HPP */