#ifndef SRC_RESOLVEDRESOURCE_HPP_
#define SRC_RESOLVEDRESOURCE_HPP_

#include "ResourceNode.hpp"
#include "ResourceParameters.hpp"

namespace httpsserver {

class ResolvedResource {
public:
	ResolvedResource();
	~ResolvedResource();

	void setMatchingNode(ResourceNode * node);
	ResourceNode * getMatchingNode();
	bool didMatch();
	ResourceParameters * getParams();
	void setParams(ResourceParameters * params);

private:
	ResourceNode * _matchingNode;
	ResourceParameters * _params;
};

} /* namespace httpsserver */

#endif /* SRC_RESOLVEDRESOURCE_HPP_ */
