#ifndef CALCULATIONMETHOD_HPP
#define CALCULATIONMETHOD_HPP

#include "CalculationParameters.hpp"

namespace CalculationMethod {

// Muslim World League
static CalculationParameters MuslimWorldLeague();

// Egyptian General Authority of Survey
static CalculationParameters Egyptian();

// University of Islamic Sciences, Karachi
static CalculationParameters Karachi();

// Umm al-Qura University, Makkah
static CalculationParameters UmmAlQura();

// Dubai
static CalculationParameters Dubai();

// Moonsighting Committee
static CalculationParameters MoonsightingCommittee();

// ISNA
static CalculationParameters NorthAmerica();

// Kuwait
static CalculationParameters Kuwait();

// Qatar
static CalculationParameters Qatar();

// Singapore
static CalculationParameters Singapore();

// Institute of Geophysics, University of Tehran
static CalculationParameters Tehran();

// Dianet
static CalculationParameters Turkey();

// Other
static CalculationParameters Other();

}; // namespace CalculationMethod

#endif /* CALCULATIONMETHOD_HPP */