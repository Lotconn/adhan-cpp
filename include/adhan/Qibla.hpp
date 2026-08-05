#ifndef QIBLA_HPP
#define QIBLA_HPP

#include "Coordinates.hpp"

namespace Adhan {

double Qibla(const Coordinates &coordinates);
const Coordinates &makkah();

} // namespace Adhan

#endif /* QIBLA_HPP */