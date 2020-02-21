#include "ResolvedResource.hpp"

namespace httpsserver {

ResolvedResource::ResolvedResource() {
  _matchingNode = NULL;
  _params = NULL;
}

ResolvedResource::~ResolvedResource() {
  // Delete only params, nodes are reused/server-internal
  if (_params != NULL) {
    delete _params;
  }
}

bool ResolvedResource::didMatch() {
  return _matchingNode != NULL;
}

HTTPNode * ResolvedResource::getMatchingNode() {
  return _matchingNode;
}

void ResolvedResource::setMatchingNode(HTTPNode * node) {
  _matchingNode = node;
}

ResourceParameters * ResolvedResource::getParams() {
  return _params;
}

void ResolvedResource::setParams(ResourceParameters * params) {
  if (_params != NULL && _params!=params) {
    delete _params;
  }
  _params = params;
}

} /* namespace httpsserver */
