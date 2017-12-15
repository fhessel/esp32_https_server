/*
 * HTTPSCallbackFunction.hpp
 *
 *  Created on: Dec 14, 2017
 *      Author: frank
 */

#ifndef HTTPS_HTTPSCALLBACKFUNCTION_HPP_
#define HTTPS_HTTPSCALLBACKFUNCTION_HPP_

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

namespace httpsserver {
	typedef void (HTTPSCallbackFunction)(HTTPRequest * req, HTTPResponse * res);
}

#endif /* HTTPS_HTTPSCALLBACKFUNCTION_HPP_ */
