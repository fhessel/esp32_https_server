#include "HTTPHeader.hpp"

#include <locale>
#include <ostream>
#include <sstream>

namespace httpsserver {

HTTPHeader::HTTPHeader(const std::string &name, const std::string &value):
  _name(normalizeHeaderName(name)),
  _value(value) {
    
}

HTTPHeader::~HTTPHeader() {

}

std::string HTTPHeader::print() {
  return _name + ": " + _value;
}

std::string normalizeHeaderName(std::string const &name) {
  std::locale loc;
  std::stringbuf buf;
  std::ostream oBuf(&buf);
  bool upper = true;
  std::string::size_type len = name.length();
  for (std::string::size_type i = 0; i < len; ++i) {
    if (upper) {
      oBuf << std::toupper(name[i], loc);
      upper = false;
    } else {
      oBuf << std::tolower(name[i], loc);
      if (!std::isalnum(name[i], loc)) {
        upper=true;
      }
    }
  }
  return buf.str();
}

} /* namespace httpsserver */
