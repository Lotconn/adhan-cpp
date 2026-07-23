#include <MathUtils.hpp>
#include <iostream>

int main() {
  std::cout << "Hi, this is some MathUtils usage:" << std::endl;
  std::cout << "👉 180 degrees to radians:\t" << degreesToRadians(180)
            << std::endl;
  std::cout << "👉 PI radians to degrees:\t" << radiansToDegrees(PI)
            << std::endl;
  return 0;
}