#include "CalculationMethod.hpp"

// Muslim World League
static CalculationParameters CalculationMethod::MuslimWorldLeague() {
  auto params = CalculationParameters("MuslimWorldLeague", 18, 17);
  params.methodAdjustments.dhuhr = 1;
  return params;
}

// Egyptian General Authority of Survey
static CalculationParameters CalculationMethod::Egyptian() {
  auto params = CalculationParameters("Egyptian", 19.5, 17.5);
  params.methodAdjustments.dhuhr = 1;
  return params;
}

// University of Islamic Sciences, Karachi
static CalculationParameters CalculationMethod::Karachi() {
  auto params = CalculationParameters("Karachi", 18, 18);
  params.methodAdjustments.dhuhr = 1;
  return params;
}

// Umm al-Qura University, Makkah
static CalculationParameters CalculationMethod::UmmAlQura() {
  return CalculationParameters("UmmAlQura", 18.5, 0, 90);
}

// Dubai
static CalculationParameters CalculationMethod::Dubai() {
  auto params = CalculationParameters("Dubai", 18.2, 18.2);
  params.methodAdjustments.sunrise = -3;
  params.methodAdjustments.dhuhr = 3;
  params.methodAdjustments.asr = 3;
  params.methodAdjustments.maghrib = 3;

  return params;
}

// Moonsighting Committee
static CalculationParameters CalculationMethod::MoonsightingCommittee() {
  auto params = CalculationParameters("MoonsightingCommittee", 18, 18);
  params.methodAdjustments.dhuhr = 5;
  params.methodAdjustments.maghrib = 3;
  return params;
}

// ISNA
static CalculationParameters CalculationMethod::NorthAmerica() {
  auto params = CalculationParameters("NorthAmerica", 15, 15);
  params.methodAdjustments.dhuhr = 1;
  return params;
}

// Kuwait
static CalculationParameters CalculationMethod::Kuwait() {
  return CalculationParameters("Kuwait", 18, 17.5);
}

// Qatar
static CalculationParameters CalculationMethod::Qatar() {
  return CalculationParameters("Qatar", 18, 0, 90);
}

// Singapore
static CalculationParameters CalculationMethod::Singapore() {
  auto params = CalculationParameters("Singapore", 20, 18);
  params.methodAdjustments.dhuhr = 1;
  params.rounding = Rounding::Up;
  return params;
}

// Institute of Geophysics, University of Tehran
static CalculationParameters CalculationMethod::Tehran() {
  auto params = CalculationParameters("Tehran", 17.7, 14, 0, 4.5);
  return params;
}

// Dianet
static CalculationParameters CalculationMethod::Turkey() {
  auto params = CalculationParameters("Turkey", 18, 17);
  params.methodAdjustments.sunrise = -7;
  params.methodAdjustments.dhuhr = 5;
  params.methodAdjustments.asr = 4;
  params.methodAdjustments.maghrib = 7;
  return params;
}

// Other
static CalculationParameters CalculationMethod::Other() {
  return CalculationParameters("Other", 0, 0);
}