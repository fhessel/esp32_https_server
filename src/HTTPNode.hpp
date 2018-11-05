#ifndef SRC_HTTPNODE_HPP_
#define SRC_HTTPNODE_HPP_

#include <Arduino.h>
#include <string>

namespace httpsserver {

enum HTTPNodeType {
	/** Node with a handler callback function (class ResourceNode) */
	HANDLER_CALLBACK,
	/** Node with a websocket handler (class WebsocketNode) */
	WEBSOCKET
};

class HTTPNode {
public:
	HTTPNode(const std::string path, const HTTPNodeType nodeType, const std::string tag = "");
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

	bool hasUrlParameter();
	uint8_t getUrlParamCount();
	size_t getParamIdx(uint8_t);

private:
	uint8_t _urlParamCount;
	size_t * _urlParamIdx;
};

} // namespace httpserver

#endif