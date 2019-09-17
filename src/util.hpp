#ifndef SRC_UTIL_HPP_
#define SRC_UTIL_HPP_

#include <Arduino.h>

#include <cmath>
#include <string>

namespace httpsserver {

/**
 * \brief **Utility function**: Parse an unsigned integer from a string
 * 
 * The second parameter can be used to define the maximum value that is acceptable
 */
uint32_t parseUInt(std::string const &s, uint32_t max = 0xffffffff);

/**
 * \brief **Utility function**: Parse a signed integer from a string
 */
int32_t parseInt(std::string const &s);

/**
 * \brief **Utility function**: Transform an int to a std::string
 */
std::string intToString(int i);

}

/**
 * \brief **Utility function**: Removes URL encoding from the string (e.g. %20 -> space)
 */
std::string urlDecode(std::string input);

#endif /* SRC_UTIL_HPP_ */
