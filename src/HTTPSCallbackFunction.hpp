#ifndef SRC_HTTPSCALLBACKFUNCTION_HPP_
#define SRC_HTTPSCALLBACKFUNCTION_HPP_

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

namespace httpsserver {
  typedef void (HTTPSCallbackFunction)(HTTPRequest * req, HTTPResponse * res);
}

#endif /* SRC_HTTPSCALLBACKFUNCTION_HPP_ */
