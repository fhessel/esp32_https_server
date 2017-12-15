/*
 * ResourceNode.hpp
 *
 *  Created on: Dec 6, 2017
 *      Author: frank
 */

#ifndef HTTPS_RESOURCENODE_HPP_
#define HTTPS_RESOURCENODE_HPP_

#include <string>

#include "HTTPSCallbackFunction.hpp"

namespace httpsserver {

class ResourceNode {
public:
	ResourceNode(const std::string path, const std::string method, const HTTPSCallbackFunction * callback);
	virtual ~ResourceNode();

	const std::string _path;
	const std::string _method;
	const HTTPSCallbackFunction * _callback;

};

} /* namespace httpsserver */

#endif /* HTTPS_RESOURCENODE_HPP_ */
