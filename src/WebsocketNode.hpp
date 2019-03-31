#ifndef SRC_WEBSOCKETNODE_HPP_
#define SRC_WEBSOCKETNODE_HPP_

#include <string>

#include "HTTPNode.hpp"
#include "WebsocketHandler.hpp"

namespace httpsserver {

typedef WebsocketHandler* (WebsocketHandlerCreator)();

class WebsocketNode : public HTTPNode {
public:
  WebsocketNode(const std::string &path, const WebsocketHandlerCreator creatorFunction, const std::string &tag = "");
  virtual ~WebsocketNode();
  WebsocketHandler* newHandler();
  std::string getMethod() { return std::string("GET"); }
private:
  const WebsocketHandlerCreator * _creatorFunction;
};

} /* namespace httpsserver */

#endif /* SRC_WEBSOCKET_HPP_ */
