/*
 * ResourceResolver.hpp
 *
 *  Created on: Dec 12, 2017
 *      Author: frank
 */

#ifndef HTTPS_RESOURCERESOLVER_HPP_
#define HTTPS_RESOURCERESOLVER_HPP_

#include <string>
// Arduino declares it's own min max, incompatible with the stl...
#undef min
#undef max
#include <vector>

#include "ResourceNode.hpp"
#include "ResolvedResource.hpp"

namespace httpsserver {

class ResourceResolver {
public:
	ResourceResolver();
	~ResourceResolver();

	void registerNode(ResourceNode *node);
	void unregisterNode(ResourceNode *node);
	void setDefaultNode(ResourceNode *node);
	void resolveNode(const std::string &method, const std::string &url, ResolvedResource &resolvedResource);
private:

	// This vector holds all nodes (with callbacks) that are registered
	std::vector<ResourceNode*> * _nodes;
	ResourceNode * _defaultNode;
};

} /* namespace httpsserver */

#endif /* HTTPS_RESOURCERESOLVER_HPP_ */
