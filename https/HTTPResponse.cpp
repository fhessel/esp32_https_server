/*
 * HTTPResponse.cpp
 *
 *  Created on: Dec 13, 2017
 *      Author: frank
 */

#include "HTTPResponse.hpp"

namespace httpsserver {

HTTPResponse::HTTPResponse(ConnectionContext * con):
	_con(con) {

	// Default status code is 200 OK
	_statusCode = 200;
	_statusText = "OK";
	_headerWritten = false;
}

HTTPResponse::~HTTPResponse() {

}

void HTTPResponse::setStatusCode(uint16_t statusCode) {
	_statusCode = statusCode;
}

void HTTPResponse::setStatusText(std::string statusText) {
	_statusText = statusText;
}

void HTTPResponse::setHeader(std::string name, std::string value) {
	_headers.set(new HTTPHeader(name, value));
}

bool HTTPResponse::isHeaderWritten() {
	return _headerWritten;
}

/**
 * Writes a string to the response. May be called several times.
 */
void HTTPResponse::printStd(const std::string &str) {
	write((uint8_t*)str.c_str(), str.length());
}

/**
 * Writes bytes to the response. May be called several times.
 */
size_t  HTTPResponse::write(const uint8_t *buffer, size_t size) {
	printHeader();
	return writeBytesInternal(buffer, size);
}

/**
 * Writes a single byte to the response.
 */
size_t  HTTPResponse::write(uint8_t b) {
	printHeader();
	byte ba[] = {b};
	return writeBytesInternal(ba, 1);
}

/**
 *  If not already done, writes the header.
 */
void HTTPResponse::printHeader() {
	if (!_headerWritten) {
		// FIXME: The usage of ostringstream increses the binary size by 200kB.
		// std::to_string is only available for the 2011 standard, but maybe
		// one can change this to a int-string-conversion this is performed manually
		std::ostringstream statusLine;
		statusLine << "HTTP/1.1" << _statusCode << " " << _statusText << "\r\n";
		printInternal(statusLine.str());
		std::vector<HTTPHeader *> * headers = _headers.getAll();
		for(std::vector<HTTPHeader*>::iterator header = headers->begin(); header != headers->end(); ++header) {
			printInternal((*header)->print()+"\r\n");
		}
		printInternal("\r\n");

		// TODO: If we wanted to use Connection: keep-alive, we need to know the content size now.

		_headerWritten=true;
	}
}

void HTTPResponse::printInternal(const std::string &str) {
	writeBytesInternal((uint8_t*)str.c_str(), str.length());
}

size_t HTTPResponse::writeBytesInternal(const void * data, int length) {
	SSL_write(_con->ssl(), data, length);
	return length;
}

} /* namespace httpsserver */
