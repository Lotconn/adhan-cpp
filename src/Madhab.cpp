#include "Madhab.hpp"

#include <stdexcept>
#include <string_view>

constexpr std::string_view to_string(Madhab m) {
  switch (m) {
  case Madhab::Shafi:
    return "Shafi";
  case Madhab::Hanafi:
    return "Hanafi";
  }
  throw std::logic_error("Invalid madhab");
  return {};
}

constexpr Madhab from_string(std::string_view s) {

  if (s == "Shafi")
    return Madhab::Shafi;
  if (s == "Hanafi")
    return Madhab::Hanafi;

  throw std::logic_error("Invalid madhab");
  return {};
}

constexpr int shadow_length(Madhab madhab) {
  switch (madhab) {
  case Madhab::Shafi:
    return 1;
  case Madhab::Hanafi:
    return 2;
  }
  throw std::logic_error("Invalid Madhab");
  return 0;
}

constexpr int shadow_length(std::string_view s) {
  return shadow_length(from_string(s));
}