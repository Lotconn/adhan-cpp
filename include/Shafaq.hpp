#ifndef SHAFAQ_HPP
#define SHAFAQ_HPP

/**
 * @brief Shafaq is the twilight in the sky. Different madhabs define the
 * appearance of twilight differently. These values are used by the
 * MoonsightingComittee method for the different ways to calculate Isha.
 */
enum class Shafaq {
  /**
   * @brief General is a combination of Ahmer and Abyad.
   */
  General,

  /**
   * @brief Ahmer means the twilight is the red glow in the sky. Used by the
   * Shafi, Maliki, and Hanbali madhabs.
   */
  Ahmer,

  /**
   * @brief Abyad means the twilight is the white glow in the sky. Used by the
   * Hanafi madhab.
   */
  Abyad,
};

#endif // SHAFAQ_HPP