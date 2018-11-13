#include "ValidatorFunctions.hpp"

namespace httpsserver {
  bool validateNotEmpty(std::string s) {
    return s!="";
  }

  bool validateUnsignedInteger(std::string s) {
    for(size_t x = 0; x < s.size(); x++) {
      if (s[x]<'0' || s[x]>'9') return false;
    }
    return true;
  }
}