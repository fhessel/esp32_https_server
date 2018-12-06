#include "HTTPURLEncodedBodyParser.hpp"

namespace httpsserver {

bool HTTPURLEncodedBodyParser::nextField() {
  return false;
}

std::string HTTPURLEncodedBodyParser::getFieldName() {
  return std::string("foo");
}

std::string HTTPURLEncodedBodyParser::getFieldMimeType() {
  return std::string("text/plain");
}

size_t HTTPURLEncodedBodyParser::getLength() {
  return 0;
}

size_t HTTPURLEncodedBodyParser::getRemainingLength() {
  return 0;
}

size_t HTTPURLEncodedBodyParser::read(byte* buffer, size_t bufferSize) {
  return 0;
}


}