#ifndef SRC_HTTPURLENCODEDBODYPARSER_HPP_
#define SRC_HTTPURLENCODEDBODYPARSER_HPP_

#include <Arduino.h>
#include "HTTPBodyParser.hpp"

namespace httpsserver {

class HTTPURLEncodedBodyParser : public HTTPBodyParser {
public:
  // From HTTPBodyParser
  HTTPURLEncodedBodyParser(HTTPRequest * req);
  ~HTTPURLEncodedBodyParser();
  virtual bool nextField();
  virtual std::string getFieldName();
  virtual std::string getFieldFilename();
  virtual std::string getFieldMimeType();
  virtual bool endOfField();
  virtual size_t read(byte* buffer, size_t bufferSize);
protected:
  char *bodyBuffer;
  char *bodyPtr;
  size_t bodyLength;
  std::string fieldName;
  std::string fieldBuffer;
  const char *fieldPtr;
  size_t fieldRemainingLength;
};

} // namespace httpserver

#endif