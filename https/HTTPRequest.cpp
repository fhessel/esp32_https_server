/*
 * HTTPRequest.cpp
 *
 *  Created on: Dec 13, 2017
 *      Author: frank
 */

#include "HTTPRequest.hpp"

namespace httpsserver {

HTTPRequest::HTTPRequest(ConnectionContext * con, HTTPHeaders * headers):
	_con(con),
	_headers(headers) {

}

HTTPRequest::~HTTPRequest() {

}

std::string HTTPRequest::getHeader(std::string name) {
	HTTPHeader * h = _headers->get(name);
	if (h != NULL) {
		return h->_value;
	} else {
		return NULL;
	}
}

} /* namespace httpsserver */
