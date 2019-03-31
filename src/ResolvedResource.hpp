#ifndef SRC_RESOLVEDRESOURCE_HPP_
#define SRC_RESOLVEDRESOURCE_HPP_

#include "ResourceNode.hpp"
#include "ResourceParameters.hpp"

namespace httpsserver {

class ResolvedResource {
public:
  ResolvedResource();
  ~ResolvedResource();

  void setMatchingNode(HTTPNode * node);
  HTTPNode * getMatchingNode();
  bool didMatch();
  ResourceParameters * getParams();
  void setParams(ResourceParameters * params);

private:
  HTTPNode * _matchingNode;
  ResourceParameters * _params;
};

} /* namespace httpsserver */

#endif /* SRC_RESOLVEDRESOURCE_HPP_ */
