#include "HTTPMultipartBodyParser.hpp"

namespace httpsserver {

bool HTTPMultipartBodyParser::nextField() {
  return false;
}

std::string HTTPMultipartBodyParser::getFieldName() {
  return std::string("foo");
}

std::string HTTPMultipartBodyParser::getFieldMimeType() {
  return std::string("text/plain");
}

size_t HTTPMultipartBodyParser::getLength() {
  return 0;
}

size_t HTTPMultipartBodyParser::getRemainingLength() {
  return 0;
}

size_t HTTPMultipartBodyParser::read(byte* buffer, size_t bufferSize) {
  return 0;
}


}