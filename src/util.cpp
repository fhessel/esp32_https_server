#include "util.hpp"

namespace httpsserver {

uint32_t parseUInt(std::string const &s, uint32_t max) {
  uint32_t i = 0; // value

  // Check sign
  size_t x = 0;
  if (s[0]=='+') {
    x = 1;
  }

  // We device max by 10, so we can check if we would exceed it by the next *10 multiplication
  max/=10;

  // Convert by base 10
  for(; x < s.size(); x++) {
    char c = s[x];
    if (i < max) {
      if (c >= '0' && c<='9') {
        i = i*10 + (c-'0');
      } else {
        break;
      }
    } else {
      return max;
    }
  }

  return i;
}

int32_t parseInt(std::string const &s) {
  uint32_t max = 0x7fffffff;
  if (s[0]=='-') {
    return -1 * parseUInt(s.substr(1,max));
  }
  return parseUInt(s,max);
}

std::string intToString(int i) {
  if (i==0) {
    return "0";
  }
  // We need this much digits
  int digits = ceil(log10(i));
  char c[digits+1];
  c[digits] = '\0';

  for(int x = digits-1; x >= 0; x--) {
    char v = (i%10);
    c[x] = '0' + v;
    i = (i-v)/10;
  }

  return std::string(c);
}

}
