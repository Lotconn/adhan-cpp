#include <adhan/MathUtils.hpp>
#include <iostream>

int main() {
  std::cout << "Hi, this is some MathUtils usage:" << std::endl;
  std::cout << "👉 180 degrees to radians:\t" << adhan::degreesToRadians(180)
            << std::endl;
  std::cout << "👉 PI radians to degrees:\t"
            << adhan::radiansToDegrees(adhan::PI) << std::endl;
  return 0;
}