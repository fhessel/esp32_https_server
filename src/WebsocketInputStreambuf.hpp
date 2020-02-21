#ifndef SRC_WEBSOCKETINPUTSTREAMBUF_HPP_
#define SRC_WEBSOCKETINPUTSTREAMBUF_HPP_

#include <Arduino.h>
#include <lwip/def.h>

#include <string>
#undef min
#undef max
#include <iostream>
#include <streambuf>
#include <sstream>

#include <iostream>

#include "HTTPSServerConstants.hpp"
#include "ConnectionContext.hpp"

namespace httpsserver {

class WebsocketInputStreambuf : public std::streambuf {
public:
  WebsocketInputStreambuf(
    ConnectionContext *con,
    size_t dataLength,
    uint8_t *_ = nullptr,
    size_t bufferSize = 2048
  );
  virtual ~WebsocketInputStreambuf();

  int_type underflow();
  void discard();
  size_t getRecordSize();

private:
  char *_buffer;
  ConnectionContext *_con;
  size_t _dataLength;
  size_t _bufferSize;
  size_t _sizeRead;
  uint8_t *_pMask;

};

} // namespace

#endif
