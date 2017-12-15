/*
 * HTTPRequest.hpp
 *
 *  Created on: Dec 13, 2017
 *      Author: frank
 */

#ifndef HTTPS_HTTPREQUEST_HPP_
#define HTTPS_HTTPREQUEST_HPP_

#include <Arduino.h>
#include <string>

#include "ConnectionContext.hpp"
#include "HTTPHeaders.hpp"
#include "HTTPHeader.hpp"

namespace httpsserver {

class HTTPRequest {
public:
	HTTPRequest(ConnectionContext * con, HTTPHeaders * headers);
	virtual ~HTTPRequest();

	std::string getHeader(std::string name);
private:
	ConnectionContext * _con;

	HTTPHeaders * _headers;
};

} /* namespace httpsserver */

#endif /* HTTPS_HTTPREQUEST_HPP_ */
