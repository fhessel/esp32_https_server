#ifndef SRC_VALIDATORFUNCTIONS_HPP_
#define SRC_VALIDATORFUNCTIONS_HPP_

#include <Arduino.h>
#include <string>
#undef max
#undef min
#include <functional>
#include <memory>
#include "HTTPValidator.hpp"
#include "util.hpp"

/**
 * This file contains some validator functions that can be used to validate URL parameters.
 * 
 * They covor common cases like checking for integer, non-empty, ..., so the user of this library
 * does not need to write them on his own.
 */

namespace httpsserver {

  /**
   * \brief **Built-in validator function**: Checks that a string is not empty.
   */
  bool validateNotEmpty(std::string s);

  /**
   * \brief **Built-in validator function**: Checks that a value is a positive int
   * 
   * Checks that the value is a positive integer (combine it with newValidateUnsignedIntegerMax if
   * you have constraints regarding the size of that number)
   */
  bool validateUnsignedInteger(std::string s);

}

#endif
