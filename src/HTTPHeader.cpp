#include "HTTPHeader.hpp"

namespace httpsserver {

HTTPHeader::HTTPHeader(const std::string &name, const std::string &value):
  _name(name),
  _value(value) {
    Serial.printf("Header Constructor: %s=%s\n4", name.c_str(), value.c_str());
}

HTTPHeader::~HTTPHeader() {

}

std::string HTTPHeader::print() {
  return _name + ": " + _value;
}

} /* namespace httpsserver */
