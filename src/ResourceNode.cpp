/*
 * ResourceNode.cpp
 *
 *  Created on: Dec 6, 2017
 *      Author: frank
 */

#include "../src/ResourceNode.hpp"

namespace httpsserver {

ResourceNode::ResourceNode(const std::string path, const std::string method, const HTTPSCallbackFunction * callback):
	_path(std::move(path)),
	_method(std::move(method)),
	_callback(callback) {

	// Count the parameters
	_urlParamCount = 0;
	size_t idx = 0;
	while((idx = path.find("/*", idx)) != std::string::npos) {
		_urlParamCount+=1;
		// If we don't do this, the same index will be found again... and again... and again...
		idx+=1;
	};

	// Check if there are parameters
	if (_urlParamCount > 0) {
		// If there are parameters, store their indices
		_urlParamIdx = new size_t[_urlParamCount];
		for(int i = 0; i < _urlParamCount; i++) {
			_urlParamIdx[i] = path.find("/*", i==0 ? 0 : _urlParamIdx[i-1])+1;
		}
	} else {
		_urlParamIdx = NULL;
	}
}

ResourceNode::~ResourceNode() {
	if (_urlParamIdx != NULL) {
		delete[] _urlParamIdx;
	}
}

bool ResourceNode::hasUrlParameter() {
	return _urlParamCount > 0;
}

size_t ResourceNode::getParamIdx(uint8_t idx) {
	if (idx<_urlParamCount) {
		return _urlParamIdx[idx];
	} else {
		return -1;
	}
}

uint8_t ResourceNode::getUrlParamCount() {
	return _urlParamCount;
}

} /* namespace httpsserver */
