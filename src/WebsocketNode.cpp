#include "WebsocketNode.hpp"

namespace httpsserver {

WebsocketNode::WebsocketNode(const std::string &path, const WebsocketHandlerCreator * creatorFunction, const std::string &tag):
  HTTPNode(path, WEBSOCKET, tag),
  _creatorFunction(creatorFunction) {

}

WebsocketNode::~WebsocketNode() {

}

WebsocketHandler* WebsocketNode::newHandler() {
  WebsocketHandler * handler = _creatorFunction();
  return handler;
}

} /* namespace httpsserver */
