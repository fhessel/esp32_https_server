#include "HTTPHeader.hpp"

namespace httpsserver {

HTTPHeader::HTTPHeader(const std::string &name, const std::string &value):
  _name(name),
  _value(value) {
    
}

HTTPHeader::~HTTPHeader() {

}

std::string HTTPHeader::print() {
  return _name + ": " + _value;
}

} /* namespace httpsserver */
