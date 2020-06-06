#ifndef SRC_RESOURCENODE_HPP_
#define SRC_RESOURCENODE_HPP_

#include <string>

#include "HTTPNode.hpp"
#include "HTTPSCallbackFunction.hpp"

namespace httpsserver {

/**
 * \brief This HTTPNode represents a route that maps to a regular HTTP request for a resource (static or dynamic)
 *
 * It therefore contrasts to the WebsocketNode, which handles requests for Websockets.
 */
class ResourceNode : public HTTPNode {
public:
  /**
   * \brief Create the Resource Node with C++-Style functional attribute.
   *
   * This variant is more flexible and allows to use std::bind for example to call class member functions.
   *
   * \param path The path/route to register the handler to, e.g. "/config"
   * \param method The method required to match this node, e.g. "GET"
   * \param callback The function to call when the route is accessed
   * \param tag Optional tag that can be accessed in the handler function. Use it for example to define the roles required to access this route
   */
  ResourceNode(const std::string &path, const std::string &method, const HTTPSCallbackFunction callback, const std::string &tag = "");
  /**
   * \brief Create the Resource Node with a C-Style function pointer.
   *
   * \param path The path/route to register the handler to, e.g. "/config"
   * \param method The method required to match this node, e.g. "GET"
   * \param callback The function callback. Must return void, first parameter is a pointer to a HTTPRequest, second a pointer to a HTTPResponse
   * \param tag Optional tag that can be accessed in the handler function. Use it for example to define the roles required to access this route
   */
  ResourceNode(const std::string &path, const std::string &method, void (*const callback)(HTTPRequest * req, HTTPResponse * res), const std::string &tag = "");
  virtual ~ResourceNode();

  const std::string _method;
  const HTTPSCallbackFunction _callback;
  std::string getMethod() { return _method; }
};

} /* namespace httpsserver */

#endif /* SRC_RESOURCENODE_HPP_ */
