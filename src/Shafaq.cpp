#include "Shafaq.hpp"

#include <cassert>
#include <stdexcept>

constexpr std::string_view to_string(Shafaq sfq) {
  switch (sfq) {

  case Shafaq::General:
    return "general";
  case Shafaq::Ahmer:
    return "ahmer";
  case Shafaq::Abyad:
    return "abyad";
  }
  throw std::logic_error("Invalid shafaq");
  return {};
}

constexpr Shafaq from_string(std::string_view s) {
  if (s == "general")
    return Shafaq::General;
  if (s == "ahmer")
    return Shafaq::Ahmer;
  if (s == "abyad")
    return Shafaq::Abyad;

  throw std::logic_error("Invalid shafaq");
  return {};
}