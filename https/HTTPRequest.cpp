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

}

std::string HTTPRequest::getHeader(std::string name) {
	HTTPHeader * h = _headers->get(name);
	if (h != NULL) {
		return h->_value;
	} else {
		return NULL;
	}
}

size_t HTTPRequest::readBytes(byte * buffer, size_t length) {

	// Limit reading to content length
	if (_contentLengthSet && length > _remainingContent) {
		length = _remainingContent;
	}

	if(length > 0 && SSL_want_read(_con->ssl())) {
		size_t res = SSL_read(_con->ssl(), buffer, length);
		if (res == 0) {
			// Connection is closed.
			// TODO: Is there _any_ way to differentiate between a closed connection and a
			// connections that's still open but that won't provide any more bytes?
			_con->signalClientClose();
			return 0;
		} else if (res == -1) {
			// An error has occured, signal this
			_con->signalRequestError();
			return -1;
		} else {
			if (_contentLengthSet) {
				_remainingContent -= res;
			}
			return res;
		}
	} else {
		// No data to read, return 0.
		return 0;
	}
}

size_t HTTPRequest::readChars(char * buffer, size_t length) {
	return 0;
}

size_t HTTPRequest::getContentLength() {
	return _remainingContent;
}

bool HTTPRequest::requestComplete() {
	return true;
}

} /* namespace httpsserver */
