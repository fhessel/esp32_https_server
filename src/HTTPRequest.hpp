#ifndef SRC_HTTPREQUEST_HPP_
#define SRC_HTTPREQUEST_HPP_

#include <Arduino.h>
#include <string>

#include <mbedtls/base64.h>

#include "ConnectionContext.hpp"
#include "HTTPHeader.hpp"
#include "HTTPHeaders.hpp"
#include "ResourceParameters.hpp"
#include "util.hpp"

namespace httpsserver {

class HTTPRequest {
public:
	HTTPRequest(ConnectionContext * con, HTTPHeaders * headers, ResourceParameters * resource, std::string requestString, std::string method, std::string tag);
	virtual ~HTTPRequest();

	std::string getHeader(std::string name);
	void setHeader(std::string name, std::string value);
	std::string getRequestString();
	std::string getMethod();
	std::string getTag();

	size_t readChars(char * buffer, size_t length);
	size_t readBytes(byte * buffer, size_t length);
	size_t getContentLength();
	bool   requestComplete();
	void   discardRequestBody();
	ResourceParameters * getParams();
	std::string getBasicAuthUser();
	std::string getBasicAuthPassword();
	bool   isSecure();
	void setWebsocketHandler(WebsocketHandler *wsHandler);
	
private:
	std::string decodeBasicAuthToken();

	ConnectionContext * _con;

	HTTPHeaders * _headers;

	ResourceParameters * _params;

	std::string _requestString;
	std::string _method;
	std::string _tag;

	bool _contentLengthSet;
	size_t _remainingContent;
};

} /* namespace httpsserver */

#endif /* SRC_HTTPREQUEST_HPP_ */
