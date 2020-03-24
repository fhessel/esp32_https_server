#ifndef SRC_HTTPBODYPARSER_HPP_
#define SRC_HTTPBODYPARSER_HPP_

#include <Arduino.h>
#include <string>
#include "HTTPRequest.hpp"

namespace httpsserver {

/**
 * Superclass for various body parser implementations that can be used to
 * interpret http-specific bodies (like x-www-form-urlencoded or multipart/form-data)
 * 
 * To allow for arbitrary body length, the interface of the body parser provides access
 * to one underlying "field" at a time. A field may be a value of the urlencoded string
 * or a part of a multipart message.
 * 
 * Using next() proceeds to the next field.
 */
class HTTPBodyParser {
public:
  const size_t unknownLength = 0x7ffffffe;

  HTTPBodyParser(HTTPRequest * req): _request(req) {};
  virtual ~HTTPBodyParser() {}

  /**
   * Proceeds to the next field of the body
   * 
   * If a field has not been read completely, the remaining content is discarded.
   * 
   * Returns true iff proceeding to the next field succeeded (ie there was a next field)
   */
  virtual bool nextField() = 0;

  /** Returns the name of the current field */
  virtual std::string getFieldName() = 0;

  /** Returns the filename of the current field or an empty string */
  virtual std::string getFieldFilename() = 0;
  
  /**
   * Returns the mime type of the current field.
   * 
   * Note: This value is set by the client. It can be altered maliciously. Do NOT rely on it
   * for anything that affects the security of your device or other clients connected to it!
   * 
   * Not every BodyParser might provide this value, usually it's set to something like text/plain then
   */
  virtual std::string getFieldMimeType() = 0;

  /** 
   * Reads a maximum of bufferSize bytes into buffer and returns the actual amount of bytes that have been read 
   */
  virtual size_t read(byte* buffer, size_t bufferSize) = 0;

  /** Returns true when all field data has been read */
  virtual bool endOfField() = 0;


protected:
  /** The underlying request */
  HTTPRequest * _request;
};

} // namespace httpserver

#endif