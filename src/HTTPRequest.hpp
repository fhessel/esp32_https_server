#ifndef SRC_HTTPREQUEST_HPP_
#define SRC_HTTPREQUEST_HPP_

#include <Arduino.h>
#include <IPAddress.h>
#include <string>

#include <mbedtls/base64.h>

#include "ConnectionContext.hpp"
#include "HTTPNode.hpp"
#include "HTTPHeader.hpp"
#include "HTTPHeaders.hpp"
#include "ResourceParameters.hpp"
#include "util.hpp"

namespace httpsserver {

/**
 * \brief Represents the request stream for an HTTP request
 */
class HTTPRequest {
public:
  HTTPRequest(ConnectionContext * con, HTTPHeaders * headers, HTTPNode * resolvedNode, std::string method, ResourceParameters * params, std::string requestString);
  virtual ~HTTPRequest();

  std::string getHeader(std::string const &name);
  void setHeader(std::string const &name, std::string const &value);
  HTTPNode * getResolvedNode();
  std::string getRequestString();
  std::string getMethod();
  std::string getTag();
  IPAddress getClientIP();

  size_t readChars(char * buffer, size_t length);
  size_t readBytes(byte * buffer, size_t length);
  size_t getContentLength();
  bool   requestComplete();
  void   discardRequestBody();
  ResourceParameters * getParams();
  HTTPHeaders *getHTTPHeaders();
  std::string getBasicAuthUser();
  std::string getBasicAuthPassword();
  bool   isSecure();
  void setWebsocketHandler(WebsocketHandler *wsHandler);

private:
  std::string decodeBasicAuthToken();

  ConnectionContext * _con;

  HTTPHeaders * _headers;

  HTTPNode * _resolvedNode;

  std::string _method;

  ResourceParameters * _params;

  std::string _requestString;

  bool _contentLengthSet;
  size_t _remainingContent;
};

} /* namespace httpsserver */

#endif /* SRC_HTTPREQUEST_HPP_ */
