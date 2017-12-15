/*
 * HTTPHeader.hpp
 *
 *  Created on: Dec 12, 2017
 *      Author: frank
 */

#ifndef HTTPS_HTTPHEADER_HPP_
#define HTTPS_HTTPHEADER_HPP_

#include <string>

namespace httpsserver {

class HTTPHeader {
public:
	HTTPHeader(const std::string name, const std::string value);
	virtual ~HTTPHeader();
	const std::string _name;
	const std::string _value;
	std::string print();
};

} /* namespace httpsserver */

#endif /* HTTPS_HTTPHEADER_HPP_ */
