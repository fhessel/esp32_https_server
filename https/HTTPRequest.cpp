/*
 * HTTPRequest.cpp
 *
 *  Created on: Dec 13, 2017
 *      Author: frank
 */

#include "HTTPRequest.hpp"

namespace httpsserver {

HTTPRequest::HTTPRequest(ConnectionContext * con, HTTPHeaders * headers, ResourceParameters * params):
	_con(con),
	_headers(headers),
	_params(params) {

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


ResourceParameters * HTTPRequest::getParams() {
	return _params;
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

	size_t bytesRead = _con->readBuffer(buffer, length);

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

bool HTTPRequest::requestComplete() {
	if (_contentLengthSet) {
		// If we have a content size, rely on it.
		return (_remainingContent == 0);
	} else {
		// If there is no more input...
		return (_con->pendingBufferSize() == 0);
	}
}

} /* namespace httpsserver */
