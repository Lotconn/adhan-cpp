#include <adhan/MathUtils.hpp>
#include <iostream>

int main() {
  std::cout << "Hi, this is some MathUtils usage:" << std::endl;
  std::cout << "👉 180 degrees to radians:\t" << Adhan::degreesToRadians(180)
            << '\n';
  std::cout << "👉 PI radians to degrees:\t"
            << Adhan::radiansToDegrees(Adhan::PI) << '\n';
  return 0;
}