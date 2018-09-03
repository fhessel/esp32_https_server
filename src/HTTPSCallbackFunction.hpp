/*
 * HTTPSCallbackFunction.hpp
 *
 *  Created on: Dec 14, 2017
 *      Author: frank
 */

#ifndef SRC_HTTPSCALLBACKFUNCTION_HPP_
#define SRC_HTTPSCALLBACKFUNCTION_HPP_

#include "../src/HTTPRequest.hpp"
#include "../src/HTTPResponse.hpp"

namespace httpsserver {
	typedef void (HTTPSCallbackFunction)(HTTPRequest * req, HTTPResponse * res);
}

#endif /* SRC_HTTPSCALLBACKFUNCTION_HPP_ */
