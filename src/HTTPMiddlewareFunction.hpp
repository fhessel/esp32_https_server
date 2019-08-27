#ifndef SRC_HTTPMIDDLEWAREFUNCTION_HPP_
#define SRC_HTTPMIDDLEWAREFUNCTION_HPP_

#include <functional>

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "HTTPSCallbackFunction.hpp"

namespace httpsserver {
  class HTTPRequest;
  /**
   * \brief A middleware function that can be registered at the server.
   *
   * It will be called before an incoming request is passed to any HTTPSCallbackFunction and may perform
   * operations like redirects or authentication.
   *
   * It receives the request and response object as well as a function pointer ("next") to pass on processing.
   * This allows chaining those functions. If next() is not called, the HTTPSCallbackFunction that
   * would match the request url will not be invoked. This might become handy if you want to intercept request
   * handling in case of missing authentication. Don't forget to call next in case you want to access your
   * resources, though.
   */
  typedef void (HTTPSMiddlewareFunction)(HTTPRequest * req, HTTPResponse * res, std::function<void()> next);
}
 #endif /* SRC_HTTPMIDDLEWAREFUNCTION_HPP_ */
