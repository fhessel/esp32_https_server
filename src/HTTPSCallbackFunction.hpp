#ifndef SRC_HTTPSCALLBACKFUNCTION_HPP_
#define SRC_HTTPSCALLBACKFUNCTION_HPP_

#include <functional>

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

namespace httpsserver {
  /**
   * \brief A callback function that will be called by the server to handle a request
   */
  typedef std::function<void(HTTPRequest *, HTTPResponse *)> HTTPSCallbackFunction;
}

#endif /* SRC_HTTPSCALLBACKFUNCTION_HPP_ */
