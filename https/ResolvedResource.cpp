/*
 * ResolvedResource.cpp
 *
 *  Created on: Dec 12, 2017
 *      Author: frank
 */

#include "ResolvedResource.hpp"

namespace httpsserver {

ResolvedResource::ResolvedResource() {
	_matchingNode = NULL;
}

ResolvedResource::~ResolvedResource() {

}

bool ResolvedResource::didMatch() {
	return _matchingNode != NULL;
}

ResourceNode * ResolvedResource::getMatchingNode() {
	return _matchingNode;
}

void ResolvedResource::setMatchingNode(ResourceNode * node) {
	_matchingNode = node;
}

} /* namespace httpsserver */
