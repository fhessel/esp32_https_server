/*
 * ResolvedResource.hpp
 *
 *  Created on: Dec 12, 2017
 *      Author: frank
 */

#ifndef HTTPS_RESOLVEDRESOURCE_HPP_
#define HTTPS_RESOLVEDRESOURCE_HPP_

#include "ResourceNode.hpp"

namespace httpsserver {

class ResolvedResource {
public:
	ResolvedResource();
	~ResolvedResource();

	void setMatchingNode(ResourceNode * node);
	ResourceNode * getMatchingNode();
	bool didMatch();

private:
	ResourceNode * _matchingNode;
};

} /* namespace httpsserver */

#endif /* HTTPS_RESOLVEDRESOURCE_HPP_ */
