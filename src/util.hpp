#ifndef SRC_UTIL_HPP_
#define SRC_UTIL_HPP_

#include <Arduino.h>

#include <cmath>
#include <string>

namespace httpsserver {

uint32_t parseUInt(std::string s, uint32_t max = 0xffffffff);

int32_t parseInt(std::string s);

std::string intToString(int i);

}

#endif /* SRC_UTIL_HPP_ */
