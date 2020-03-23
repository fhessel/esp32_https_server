#include "HTTPResponse.hpp"

#include <Arduino.h>
#include "lwip/sockets.h"

namespace httpsserver {

HTTPResponse::HTTPResponse(ConnectionContext * con):
  _con(con) {

  // Default status code is 200 OK
  _statusCode = 200;
  _statusText = "OK";
  _headerWritten = false;
  _isError = false;

  _responseCacheSize = con->getCacheSize();
  _responseCachePointer = 0;
  if (_responseCacheSize > 0) {
    HTTPS_LOGD("Creating buffered response, size: %d", _responseCacheSize);
    _responseCache = new byte[_responseCacheSize];
  } else {
    HTTPS_LOGD("Creating non-buffered response");
    _responseCache = NULL;
  }
}

HTTPResponse::~HTTPResponse() {
  if (_responseCache != NULL) {
    delete[] _responseCache;
  }
  _headers.clearAll();
}

void HTTPResponse::setStatusCode(uint16_t statusCode) {
  _statusCode = statusCode;
}

void HTTPResponse::setStatusText(std::string const &statusText) {
  _statusText = statusText;
}

uint16_t HTTPResponse::getStatusCode() {
  return _statusCode;
}

std::string HTTPResponse::getStatusText() {
  return _statusText;
}

void HTTPResponse::setHeader(std::string const &name, std::string const &value) {
  _headers.set(new HTTPHeader(name, value));
}

std::string HTTPResponse::getHeader(std::string const &name) {
  HTTPHeader * h = _headers.get(name);
  if (h != NULL) {
    return h->_value;
  } else {
    return std::string();
  }
}

bool HTTPResponse::isHeaderWritten() {
  return _headerWritten;
}

bool HTTPResponse::isResponseBuffered() {
  return _responseCache != NULL;
}

void HTTPResponse::finalize() {
  if (isResponseBuffered()) {
    drainBuffer();
  }
}

/**
 * Writes a string to the response. May be called several times.
 */
void HTTPResponse::printStd(const std::string &str) {
  write((uint8_t*)str.c_str(), str.length());
}

/**
 * Writes bytes to the response. May be called several times.
 */
size_t  HTTPResponse::write(const uint8_t *buffer, size_t size) {
  if(!isResponseBuffered()) {
    printHeader();
  }
  return writeBytesInternal(buffer, size);
}

/**
 * Writes a single byte to the response.
 */
size_t  HTTPResponse::write(uint8_t b) {
  if(!isResponseBuffered()) {
    printHeader();
  }
  byte ba[] = {b};
  return writeBytesInternal(ba, 1);
}

/**
 *  If not already done, writes the header.
 */
void HTTPResponse::printHeader() {
  if (!_headerWritten) {
    HTTPS_LOGD("Printing headers");

    // Status line, like: "HTTP/1.1 200 OK\r\n"
    std::string statusLine = "HTTP/1.1 " + intToString(_statusCode) + " " + _statusText + "\r\n";
    printInternal(statusLine, true);

    // Each header, like: "Host: myEsp32\r\n"
    std::vector<HTTPHeader *> * headers = _headers.getAll();
    for(std::vector<HTTPHeader*>::iterator header = headers->begin(); header != headers->end(); ++header) {
      printInternal((*header)->print()+"\r\n", true);
    }
    printInternal("\r\n", true);

    _headerWritten=true;
  }
}

/**
 * This method can be called to cancel the ongoing transmission and send the error page (if possible)
 */
void HTTPResponse::error() {
  _con->signalRequestError();
}

void HTTPResponse::printInternal(const std::string &str, bool skipBuffer) {
  writeBytesInternal((uint8_t*)str.c_str(), str.length(), skipBuffer);
}

size_t HTTPResponse::writeBytesInternal(const void * data, int length, bool skipBuffer) {
  if (!_isError) {
    if (isResponseBuffered() && !skipBuffer) {
      // We are buffering ...
      if(length <= _responseCacheSize - _responseCachePointer) {
        // ... and there is space left in the buffer -> Write to buffer
        size_t end = _responseCachePointer + length;
        size_t i = 0;
        while(_responseCachePointer < end) {
          _responseCache[_responseCachePointer++] = ((byte*)data)[i++];
        }
        // Returning skips the SSL_write below
        return length;
      } else {
        // .., and the buffer is too small. This is the point where we switch from
        // caching to streaming
        if (!_headerWritten) {
          setHeader("Connection", "close");
        }
        drainBuffer(true);
      }
    }

    return _con->writeBuffer((byte*)data, length);
  } else {
    return 0;
  }
}

void HTTPResponse::drainBuffer(bool onOverflow) {
  if (!_headerWritten) {
    if (_responseCache != NULL && !onOverflow) {
      _headers.set(new HTTPHeader("Content-Length", intToString(_responseCachePointer)));
    }
    printHeader();
  }

  if (_responseCache != NULL) {
    HTTPS_LOGD("Draining response buffer");
    // Check for 0 as it may be an overflow reaction without any data that has been written earlier
    if(_responseCachePointer > 0) {
      // FIXME: Return value?
      _con->writeBuffer((byte*)_responseCache, _responseCachePointer);
    }
    delete[] _responseCache;
    _responseCache = NULL;
  }
}

} /* namespace httpsserver */
