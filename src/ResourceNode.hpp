#ifndef SRC_RESOURCENODE_HPP_
#define SRC_RESOURCENODE_HPP_

#include <string>

#include "HTTPNode.hpp"
#include "HTTPSCallbackFunction.hpp"

namespace httpsserver {

class ResourceNode : public HTTPNode {
public:
  ResourceNode(const std::string &path, const std::string &method, const HTTPSCallbackFunction * callback, const std::string &tag = "");
  virtual ~ResourceNode();

  const std::string _method;
  const HTTPSCallbackFunction * _callback;
  std::string getMethod() { return _method; }
};

} /* namespace httpsserver */

#endif /* SRC_RESOURCENODE_HPP_ */
