#include "HTTPURLEncodedBodyParser.hpp"

#define CHUNKSIZE 512
#define MINCHUNKSIZE 64

namespace httpsserver {

HTTPURLEncodedBodyParser::HTTPURLEncodedBodyParser(HTTPRequest * req):
  HTTPBodyParser(req),
  bodyBuffer(NULL),
  bodyPtr(NULL),
  bodyLength(0),
  fieldBuffer(""),
  fieldPtr(NULL),
  fieldRemainingLength(0)
{
  bodyLength = _request->getContentLength();
  if (bodyLength) {
    // We know the body length. We try to read that much and give an error if it fails.
    bodyBuffer = (char *)malloc(bodyLength+1);
    if (bodyBuffer == NULL) {
      HTTPS_LOGE("HTTPURLEncodedBodyParser: out of memory");
      return;
    }
    bodyPtr = bodyBuffer;
    size_t toRead = bodyLength;
    while(toRead > 0) {
      size_t didRead = _request->readChars(bodyPtr, toRead);
      if (didRead == 0) {
        HTTPS_LOGE("HTTPURLEncodedBodyParser: short read");
        bodyLength = bodyPtr - bodyBuffer;
        break;
      }
      bodyPtr += didRead;
      toRead -= didRead;
    }
  } else {
    // We don't know the length. Read as much as possible.
    bodyBuffer = (char *)malloc(CHUNKSIZE+1);
    if (bodyBuffer == NULL) {
      HTTPS_LOGE("HTTPURLEncodedBodyParser: out of memory");
      return;
    }
    bodyPtr = bodyBuffer;
    size_t bufferUsed = 0;
    size_t bufferAvailable = CHUNKSIZE;
    while(!_request->requestComplete()) {
      if (bufferAvailable < MINCHUNKSIZE) {
        char *pBuf = (char *)realloc(bodyBuffer, bufferUsed + CHUNKSIZE+1);
        if (pBuf == NULL) {
          HTTPS_LOGE("HTTPURLEncodedBodyParser: out of memory");
          free(bodyBuffer);
          bodyBuffer = NULL;
          return;
        }
        bodyBuffer = pBuf;
        bufferAvailable = CHUNKSIZE;
      }
      size_t didRead = _request->readChars(bodyBuffer+bufferUsed, bufferAvailable);
      bufferUsed += didRead;
      bufferAvailable -= didRead;
    }
    bodyLength = bufferUsed;
  }
  bodyPtr = bodyBuffer;
  bodyBuffer[bodyLength] = '\0';
}

HTTPURLEncodedBodyParser::~HTTPURLEncodedBodyParser() {
  if (bodyBuffer) {
    free(bodyBuffer);
  }
  bodyBuffer = NULL;
}

bool HTTPURLEncodedBodyParser::nextField() {
  fieldBuffer = "";
  fieldPtr = NULL;
  fieldRemainingLength = 0;

  char *equalPtr = index(bodyPtr, '=');
  if (equalPtr == NULL) {
    return false;
  }
  fieldName = std::string(bodyPtr, equalPtr-bodyPtr);
  
  char *valuePtr = equalPtr + 1;
  char *endPtr = index(valuePtr, '&');
  if (endPtr == NULL) {
    endPtr = equalPtr + strlen(equalPtr);
    bodyPtr = endPtr;
  } else {
    bodyPtr = endPtr+1;
  }
  fieldBuffer = std::string(valuePtr, endPtr - valuePtr);
  fieldBuffer = urlDecode(fieldBuffer);
  fieldRemainingLength = fieldBuffer.size();
  fieldPtr = fieldBuffer.c_str();
  return true;
}

std::string HTTPURLEncodedBodyParser::getFieldName() {
  return fieldName;
}

std::string HTTPURLEncodedBodyParser::getFieldFilename() {
  return "";
}

std::string HTTPURLEncodedBodyParser::getFieldMimeType() {
  return std::string("text/plain");
}

bool HTTPURLEncodedBodyParser::endOfField() {
  return fieldRemainingLength <= 0;
}

size_t HTTPURLEncodedBodyParser::read(byte* buffer, size_t bufferSize) {
  if (bufferSize > fieldRemainingLength) {
    bufferSize = fieldRemainingLength;
  }
  memcpy(buffer, fieldPtr, bufferSize);
  fieldRemainingLength -= bufferSize;
  fieldPtr += bufferSize;
  return bufferSize;
}

} /* namespace httpsserver */
