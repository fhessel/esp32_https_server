/*
 * ResourceNode.cpp
 *
 *  Created on: Dec 6, 2017
 *      Author: frank
 */

#include "ResourceNode.hpp"

namespace httpsserver {

ResourceNode::ResourceNode(const std::string path, const std::string method, const HTTPSCallbackFunction * callback):
	_path(std::move(path)),
	_method(std::move(method)),
	_callback(callback) {


}

ResourceNode::~ResourceNode() {

}

} /* namespace httpsserver */
