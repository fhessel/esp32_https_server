/*
 * ResourceResolver.cpp
 *
 *  Created on: Dec 13, 2017
 *      Author: frank
 */

#include "ResourceResolver.hpp"

namespace httpsserver {

ResourceResolver::ResourceResolver() {
	_nodes = new std::vector<ResourceNode *>();
	_defaultNode = NULL;
}

ResourceResolver::~ResourceResolver() {
	delete _nodes;
}

/**
 * This method will register the HTTPSNode so it is reachable and its callback gets called for a request
 */
void ResourceResolver::registerNode(ResourceNode *node) {
	_nodes->push_back(node);
}

/**
 * This method can be used to deactivate a HTTPSNode that has been registered previously
 */
void ResourceResolver::unregisterNode(ResourceNode *node) {

}

void ResourceResolver::resolveNode(const std::string &method, const std::string &url, ResolvedResource &resolvedResource) {
	// Reset the resource
	resolvedResource.setMatchingNode(NULL);

	// Check whether a resource matches
	for(std::vector<ResourceNode*>::iterator node = _nodes->begin(); node != _nodes->end(); ++node) {
	    if ((*node)->_method == method, (*node)->_path == url) {
	    	resolvedResource.setMatchingNode(*node);
	    	break;
	    }
	}

	// If the resource did not match, configure the default resource
	if (!resolvedResource.didMatch() && _defaultNode != NULL) {
		resolvedResource.setMatchingNode(_defaultNode);
	}
}

void ResourceResolver::setDefaultNode(ResourceNode * defaultNode) {
	_defaultNode = defaultNode;
}

}
