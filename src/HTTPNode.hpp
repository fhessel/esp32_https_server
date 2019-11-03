#ifndef SRC_HTTPNODE_HPP_
#define SRC_HTTPNODE_HPP_

#include <Arduino.h>
#include <string>
#undef min
#undef max
#include <vector>
#include "HTTPValidator.hpp"

namespace httpsserver {

enum HTTPNodeType {
  /** Node with a handler callback function (class ResourceNode) */
  HANDLER_CALLBACK,
  /** Node with a websocket handler (class WebsocketNode) */
  WEBSOCKET
};

/**
 * \brief Base class for a URL/route-handler in the server.
 * 
 * Use ResourceNode for requests that access dynamic or static resources or HttpNode for routes that
 * create Websockets.
 */
class HTTPNode {
public:
  HTTPNode(const std::string &path, const HTTPNodeType nodeType, const std::string &tag = "");
  virtual ~HTTPNode();

  /**
   * The path under which this node will be available. Should start with a slash. Example:
   * "/myResource"
   */
  const std::string _path;

  /**
   * Stores a tag that can be used in middleware and handler functions to identify this
   * specific node, tag the node with a required permission, ...
   */
  const std::string _tag;

  /** Stores the type of the node (as we have not runtime type information by default) */
  const HTTPNodeType _nodeType;

  bool hasPathParameter();
  size_t getPathParamCount();
  ssize_t getParamIdx(size_t);

  std::vector<HTTPValidator*> * getValidators();

  virtual std::string getMethod() = 0;

  /**
   * Adds a validation function that checks if the actual value of a parameter matches the expectation
   * @param paramIdx defines the ID of the parameter that should be checked (starts by 0)
   * @param validator the function (string -> bool) that checks if the parameter matches the expecatation
   * 
   * @see ValidatorFunctions.hpp if you need some predefined templates for functions
   */
  void addPathParamValidator(size_t paramIdx, const HTTPValidationFunction * validator);

private:
  std::vector<size_t> _pathParamIdx;
  std::vector<HTTPValidator*> _validators;
};

} // namespace httpserver

#endif
