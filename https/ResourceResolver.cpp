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
	resolvedResource.setParams(NULL);

	// Memory management of this object will be performed by the ResolvedResource instance
	ResourceParameters * params = new ResourceParameters();

	// TODO: Split URL in resource name and request params, set request params in params object
	std::string resourceName = url;


	// Check whether a resource matches
	for(std::vector<ResourceNode*>::iterator node = _nodes->begin(); node != _nodes->end(); ++node) {
		params->resetUrlParameters();
		if ((*node)->_method == method) {
			const std::string nodepath = ((*node)->_path);
			if (!((*node)->hasUrlParameter())) {
				std::string logstring = "[   ] Testing simple match on " + nodepath;
				HTTPS_DLOG(logstring.c_str())

				// Simple matching, the node does not contain any resource parameters
				if (nodepath == resourceName) {
					resolvedResource.setMatchingNode(*node);
					HTTPS_DLOG("[   ]   It's a match!")
					break;
				}
			} else {
				std::string logstring = "[   ] Testing parameter match on " + nodepath;
				HTTPS_DLOG(logstring.c_str())

				// Advanced matching, we need to align the /?/ parts.
				bool didMatch = true;
				size_t urlIdx = 0; // Pointer how far the input url is processed
				size_t nodeIdx = 0; // Pointer how far the node url is processed
				for (int pIdx = 0; didMatch && pIdx < (*node)->getUrlParamCount(); pIdx++) {
					size_t pOffset = (*node)->getParamIdx(pIdx);

					// First step: Check static part
					size_t staticLength = pOffset-nodeIdx;
					if (nodepath.substr(nodeIdx, staticLength).compare(resourceName.substr(urlIdx, staticLength))!=0) {
						// static part did not match
						didMatch = false;
						HTTPS_DLOGHEX("[   ]   No match on static part", pIdx)
					} else {
						// static part did match, increase pointers
						nodeIdx += staticLength + 1; // +1 to take care of the '*' placeholder.
						urlIdx  += staticLength; // The pointer should now point to the begin of the static part

						// Second step: Grab the parameter value
						if (nodeIdx == nodepath.length()) {
							// Easy case: parse until end of string
							params->setUrlParameter(pIdx, resourceName.substr(urlIdx));
						} else {
							// parse until first char after the placeholder
							char terminatorChar = nodepath[nodeIdx];
							size_t terminatorPosition = resourceName.find(terminatorChar, urlIdx);
							if (terminatorPosition != std::string::npos) {
								// We actually found the terminator
								size_t dynamicLength = terminatorPosition-urlIdx;
								params->setUrlParameter(pIdx, resourceName.substr(urlIdx, dynamicLength));
								urlIdx = urlIdx + dynamicLength;
							} else {
								// We did not find the terminator
								didMatch = false;
								HTTPS_DLOGHEX("[   ]   No match on dynamic part", pIdx)
							}
						}
					} // static part did match
				} // placeholder loop

				// If there is some final static part to process
				if (didMatch && nodeIdx < nodepath.length()) {
					size_t staticLength = nodepath.length()-nodeIdx;
					if (nodepath.substr(nodeIdx, staticLength).compare(url.substr(urlIdx, staticLength))!=0) {
						didMatch = false;
						HTTPS_DLOG("[   ]   No match, final static part did not match")
					} else {
						urlIdx += staticLength;
						// If there is some string remaining in the url that did not match
						if (urlIdx < resourceName.length()) {
							didMatch = false;
							HTTPS_DLOG("[   ]   No match, url is longer than final static part")
						}
					}
				}

				// Every check worked, so the full url matches and the params are set
				if (didMatch) {
					resolvedResource.setMatchingNode(*node);
					HTTPS_DLOG("[   ]   It's a match!")
					break;
				}

			} // static/dynamic url
		} // method check
	} // resource node for loop

	// If the resource did not match, configure the default resource
	if (!resolvedResource.didMatch() && _defaultNode != NULL) {
		params->resetUrlParameters();
		resolvedResource.setMatchingNode(_defaultNode);
	}

	// If resolving did work, set the params, otherwise delete them
	if (resolvedResource.didMatch()) {
		// The resolvedResource now takes care of memory management for the params
		resolvedResource.setParams(params);
	} else {
		delete params;
	}
}

void ResourceResolver::setDefaultNode(ResourceNode * defaultNode) {
	_defaultNode = defaultNode;
}

}
