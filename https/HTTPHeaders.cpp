/*
 * HTTPHeaders.cpp
 *
 *  Created on: Dec 13, 2017
 *      Author: frank
 */

#include "HTTPHeaders.hpp"

namespace httpsserver {

HTTPHeaders::HTTPHeaders() {
	_headers = new std::vector<HTTPHeader *>();

}

HTTPHeaders::~HTTPHeaders() {
	for(std::vector<HTTPHeader*>::iterator header = _headers->begin(); header != _headers->end(); ++header) {
		delete (*header);
	}
	delete _headers;
}

HTTPHeader * HTTPHeaders::get(const std::string name) {
	for(std::vector<HTTPHeader*>::iterator header = _headers->begin(); header != _headers->end(); ++header) {
		if ((*header)->_name.compare(name)==0) {
			return (*header);
		}
	}
	return NULL;
}

void HTTPHeaders::set(HTTPHeader * header) {
	for(int i = 0; i < _headers->size(); i++) {
		if ((*_headers)[i]->_name.compare(header->_name)==0) {
			delete (*_headers)[i];
			(*_headers)[i] = header;
			return;
		}
	}
	_headers->push_back(header);
}

std::vector<HTTPHeader *> * HTTPHeaders::getAll() {
	return _headers;
}

} /* namespace httpsserver */
