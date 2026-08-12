// This program links against the installed package.
// Successfully compiling and linking is the whole
// point of this check
#include <adhan/Qibla.hpp>
#include <iostream>
int main() {
  std::cout << Adhan::qibla(Adhan::makkah()) << '\n';
  return 0;
}