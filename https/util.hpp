/*
 * util.h
 *
 *  Created on: Dec 17, 2017
 *      Author: frank
 */

#ifndef HTTPS_UTIL_HPP_
#define HTTPS_UTIL_HPP_

#include <Arduino.h>

#include <cmath>
#include <string>

namespace httpsserver {

int parseInt(std::string s);

std::string intToString(int i);

}

#endif /* HTTPS_UTIL_HPP_ */
