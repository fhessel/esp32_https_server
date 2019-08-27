#ifndef SRC_RESOURCERESOLVER_HPP_
#define SRC_RESOURCERESOLVER_HPP_

#include <string>
// Arduino declares it's own min max, incompatible with the stl...
#undef min
#undef max
#include <vector>
#include <algorithm>

#include "HTTPNode.hpp"
#include "WebsocketNode.hpp"
#include "ResourceNode.hpp"
#include "ResolvedResource.hpp"
#include "HTTPMiddlewareFunction.hpp"

namespace httpsserver {

/**
 * \brief This class is used internally to resolve a string URL to the corresponding HTTPNode
 */
class ResourceResolver {
public:
  ResourceResolver();
  ~ResourceResolver();

  void registerNode(HTTPNode *node);
  void unregisterNode(HTTPNode *node);
  void setDefaultNode(HTTPNode *node);
  void resolveNode(const std::string &method, const std::string &url, ResolvedResource &resolvedResource, HTTPNodeType nodeType);

  /** Add a middleware function to the end of the middleware function chain. See HTTPSMiddlewareFunction.hpp for details. */
  void addMiddleware(const HTTPSMiddlewareFunction * mwFunction);
  /** Remove a specific function from the middleware function chain. */
  void removeMiddleware(const HTTPSMiddlewareFunction * mwFunction);
  /** Get the current middleware chain with a resource function at the end */
  const std::vector<HTTPSMiddlewareFunction*> getMiddleware();

private:

  // This vector holds all nodes (with callbacks) that are registered
  std::vector<HTTPNode*> * _nodes;
  HTTPNode * _defaultNode;

  // Middleware functions, if any are registered. Will be called in order of the vector.
  std::vector<const HTTPSMiddlewareFunction*> _middleware;
};

} /* namespace httpsserver */

#endif /* SRC_RESOURCERESOLVER_HPP_ */
