#include "HTTPRequest.hpp"

namespace httpsserver {

HTTPRequest::HTTPRequest(
    ConnectionContext * con,
    HTTPHeaders * headers,
    HTTPNode * resolvedNode,
    std::string method,
    ResourceParameters * params,
    std::string requestString):
  _con(con),
  _headers(headers),
  _resolvedNode(resolvedNode),
  _method(method),
  _params(params),
  _requestString(requestString) {

  HTTPHeader * contentLength = headers->get("Content-Length");
  if (contentLength == NULL) {
    _remainingContent = 0;
    _contentLengthSet = false;
  } else {
    _remainingContent = parseInt(contentLength->_value);
    _contentLengthSet = true;
  }

}

HTTPRequest::~HTTPRequest() {
  _headers->clearAll();
}


ResourceParameters * HTTPRequest::getParams() {
  return _params;
}

HTTPHeaders * HTTPRequest::getHTTPHeaders() {
  return _headers;
}

std::string HTTPRequest::getHeader(std::string const &name) {
  HTTPHeader * h = _headers->get(name);
  if (h != NULL) {
    return h->_value;
  } else {
    return std::string();
  }
}

void HTTPRequest::setHeader(std::string const &name, std::string const &value) {
  _headers->set(new HTTPHeader(name, value));
}

HTTPNode * HTTPRequest::getResolvedNode() {
  return _resolvedNode;
}

IPAddress HTTPRequest::getClientIP() {
  return _con->getClientIP();
}

size_t HTTPRequest::readBytes(byte * buffer, size_t length) {

  // Limit reading to content length
  if (_contentLengthSet && length > _remainingContent) {
    length = _remainingContent;
  }

  size_t bytesRead = 0;
  if (length > 0) {
    bytesRead = _con->readBuffer(buffer, length);
  }

  if (_contentLengthSet) {
    _remainingContent -= bytesRead;
  }

  return bytesRead;
}

size_t HTTPRequest::readChars(char * buffer, size_t length) {
  return readBytes((byte*)buffer, length);
}

size_t HTTPRequest::getContentLength() {
  return _remainingContent;
}

std::string HTTPRequest::getRequestString() {
  return _requestString;
}

std::string HTTPRequest::getMethod() {
  return _method;
}

std::string HTTPRequest::getTag() {
  return _resolvedNode->_tag;
}

bool HTTPRequest::requestComplete() {
  if (_contentLengthSet) {
    // If we have a content size, rely on it.
    return (_remainingContent == 0);
  } else {
    // If there is no more input...
    return (_con->pendingBufferSize() == 0);
  }
}

/**
 * This function will drop whatever is remaining of the request body
 */
void HTTPRequest::discardRequestBody() {
  byte buf[16];
  while(!requestComplete()) {
    readBytes(buf, 16);
  }
}

std::string HTTPRequest::getBasicAuthUser() {
  std::string token = decodeBasicAuthToken();
  size_t splitpoint = token.find(":");
  if (splitpoint != std::string::npos && splitpoint > 0) {
    return token.substr(0, splitpoint);
  } else {
    return std::string();
  }
}

std::string HTTPRequest::getBasicAuthPassword() {
  std::string token = decodeBasicAuthToken();
  size_t splitpoint = token.find(":");
  if (splitpoint != std::string::npos && splitpoint > 0) {
    return token.substr(splitpoint+1);
  } else {
    return std::string();
  }
}

std::string HTTPRequest::decodeBasicAuthToken() {
  std::string basicAuthString = getHeader("Authorization");
  // Get the length of the token
  size_t sourceLength = basicAuthString.length();
  // Only handle basic auth tokens
  if (basicAuthString.substr(0, 6) != "Basic ") {
    return std::string();
  }
  // If the token is too long, skip
  if (sourceLength > 100) {
    return std::string();
  } else {
    // Try to decode. As we are using mbedtls anyway, we can use that function
    unsigned char * bufOut = new unsigned char[basicAuthString.length()];
    size_t outputLength = 0;
    int res = mbedtls_base64_decode(
        bufOut,
        sourceLength,
        &outputLength,
        ((const unsigned char *)basicAuthString.substr(6).c_str()), // Strip "Basic "
        sourceLength - 6 // Strip "Basic "
    );
    // Failure of decoding
    if (res != 0) {
      delete[] bufOut;
      return std::string();
    }
    std::string tokenRes = std::string((char*)bufOut, outputLength);
    delete[] bufOut;
    return tokenRes;
  }
}

bool HTTPRequest::isSecure() {
  return _con->isSecure();
}


void HTTPRequest::setWebsocketHandler(WebsocketHandler *wsHandler) {
  _con->setWebsocketHandler(wsHandler);
}

} /* namespace httpsserver */
