#ifndef SRC_UTIL_HPP_
#define SRC_UTIL_HPP_

#include <Arduino.h>

#include <cmath>
#include <string>

namespace httpsserver {

int parseInt(std::string s);

std::string intToString(int i);

int base64EncodedLength(size_t length);
int base64EncodedLength(const std::string &in);
void a3_to_a4(unsigned char * a4, unsigned char * a3);
bool base64Encode(const std::string& in, std::string* out);

}

#endif /* SRC_UTIL_HPP_ */
