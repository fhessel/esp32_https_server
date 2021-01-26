#include "ResourceNode.hpp"

namespace httpsserver {

ResourceNode::ResourceNode(const std::string &path, const std::string &method, const HTTPSCallbackFunction callback, const std::string &tag):
  HTTPNode(path, HANDLER_CALLBACK, tag),
  _method(method),
  _callback(callback) {

}

ResourceNode::ResourceNode(const std::string &path, const std::string &method, void (*callback)(HTTPRequest * req, HTTPResponse * res), const std::string &tag):
  HTTPNode(path, HANDLER_CALLBACK, tag),
  _method(method),
  _callback(HTTPSCallbackFunction(callback)) {

}

ResourceNode::~ResourceNode() {
  
}

} /* namespace httpsserver */
