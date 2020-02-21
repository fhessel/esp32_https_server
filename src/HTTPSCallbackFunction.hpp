#ifndef SRC_HTTPSCALLBACKFUNCTION_HPP_
#define SRC_HTTPSCALLBACKFUNCTION_HPP_

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

namespace httpsserver {
  /**
   * \brief A callback function that will be called by the server to handle a request
   */
  typedef void (HTTPSCallbackFunction)(HTTPRequest * req, HTTPResponse * res);
}

#endif /* SRC_HTTPSCALLBACKFUNCTION_HPP_ */
