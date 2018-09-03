/*
 * HTTPHeader.cpp
 *
 *  Created on: Dec 12, 2017
 *      Author: frank
 */

#include "../src/HTTPHeader.hpp"

namespace httpsserver {

HTTPHeader::HTTPHeader(const std::string name, const std::string value):
	_name(std::move(name)),
	_value(std::move(value)) {

}

HTTPHeader::~HTTPHeader() {

}

std::string HTTPHeader::print() {
	return _name + ": " + _value;
}

} /* namespace httpsserver */
