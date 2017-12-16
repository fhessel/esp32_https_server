/*
 * HTTPResponse.hpp
 *
 *  Created on: Dec 13, 2017
 *      Author: frank
 */

#ifndef HTTPS_HTTPRESPONSE_HPP_
#define HTTPS_HTTPRESPONSE_HPP_

#include <Arduino.h>
#include <sstream>
#include <string>
// Arduino declares it's own min max, incompatible with the stl...
#undef min
#undef max
#include <vector>

#include <openssl/ssl.h>

#include "ConnectionContext.hpp"
#include "HTTPHeaders.hpp"
#include "HTTPHeader.hpp"

namespace httpsserver {

class HTTPResponse : public Print {
public:
	HTTPResponse(ConnectionContext * con);
	virtual ~HTTPResponse();

	void setStatusCode(uint16_t statusCode);
	void setStatusText(std::string statusText);
	void setHeader(std::string name, std::string value);
	bool isHeaderWritten();

	void printStd(const std::string &str);

	// From Print:
	size_t write(const uint8_t *buffer, size_t size);
	size_t write(uint8_t);
private:
	void printHeader();
	void printInternal(const std::string &str);
	size_t writeBytesInternal(const void * data, int length);

	ConnectionContext * _con;

	uint16_t _statusCode;
	std::string _statusText;
	HTTPHeaders _headers;
	bool _headerWritten;
};

} /* namespace httpsserver */

#endif /* HTTPS_HTTPRESPONSE_HPP_ */
