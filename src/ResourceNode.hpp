#ifndef SRC_RESOURCENODE_HPP_
#define SRC_RESOURCENODE_HPP_

#include <string>

#include "HTTPSCallbackFunction.hpp"

namespace httpsserver {

class ResourceNode {
public:
	ResourceNode(const std::string path, const std::string method, const HTTPSCallbackFunction * callback, const std::string tag = "");
	virtual ~ResourceNode();

	const std::string _path;
	const std::string _method;
	const std::string _tag;
	const HTTPSCallbackFunction * _callback;

	bool hasUrlParameter();
	uint8_t getUrlParamCount();
	size_t getParamIdx(uint8_t);

private:
	uint8_t _urlParamCount;
	size_t * _urlParamIdx;
};

} /* namespace httpsserver */

#endif /* SRC_RESOURCENODE_HPP_ */
