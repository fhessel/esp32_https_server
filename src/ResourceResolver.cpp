#include "ResourceResolver.hpp"

namespace httpsserver {

ResourceResolver::ResourceResolver() {
  _nodes = new std::vector<HTTPNode *>();
  _defaultNode = NULL;
}

ResourceResolver::~ResourceResolver() {
  delete _nodes;
}

/**
 * This method will register the HTTPSNode so it is reachable and its callback gets called for a request
 */
void ResourceResolver::registerNode(HTTPNode *node) {
  _nodes->push_back(node);
}

/**
 * This method can be used to deactivate a HTTPSNode that has been registered previously
 */
void ResourceResolver::unregisterNode(HTTPNode *node) {

}

void ResourceResolver::resolveNode(const std::string &method, const std::string &url, ResolvedResource &resolvedResource, HTTPNodeType nodeType) {
  // Reset the resource
  resolvedResource.setMatchingNode(NULL);
  resolvedResource.setParams(NULL);

  // Memory management of this object will be performed by the ResolvedResource instance
  ResourceParameters * params = new ResourceParameters();

  // Split URL in resource name and request params. Request params start after an optional '?'
  size_t reqparamIdx = url.find('?');

  // If no '?' is contained in url, 0:npos will return the string as it is
  std::string resourceName = url.substr(0, reqparamIdx);

  // Set request params in params object if a '?' exists
  if (reqparamIdx != std::string::npos) {
    do {
      // Drop the '?' or '&'
      reqparamIdx += 1;

      // Parameters are separated by '&'
      size_t nextparamIdx = url.find('&', reqparamIdx);

      // Get the "name=value" string
      std::string param = url.substr(reqparamIdx, nextparamIdx - reqparamIdx);

      // Find the position where the string has to be split
      size_t nvSplitIdx = param.find('=');

      // Use empty string if only name is set. /foo?bar&baz=1 will return "" for bar
      std::string name  = param.substr(0, nvSplitIdx);
      std::string value = "";
      if (nvSplitIdx != std::string::npos) {
        value = urlDecode(param.substr(nvSplitIdx+1));
      }

      // Now we finally have name and value.
      params->setRequestParameter(name, value);

      // Update reqparamIdx
      reqparamIdx = nextparamIdx;

    } while(reqparamIdx != std::string::npos);
  }


  // Check whether a resource matches
  for(std::vector<HTTPNode*>::iterator node = _nodes->begin(); node != _nodes->end(); ++node) {
    params->resetUrlParameters();
    if ((*node)->_nodeType==nodeType) {
      if (
        // For handler functions, check the method declared with the node
        ((*node)->_nodeType==HANDLER_CALLBACK && ((ResourceNode*)*node)->_method == method) ||
        // For websockets, the specification says that GET is the only choice
        ((*node)->_nodeType==WEBSOCKET && method=="GET")
      ) {
        const std::string nodepath = ((*node)->_path);
        if (!((*node)->hasUrlParameter())) {
          HTTPS_LOGD("Testing simple match on %s", nodepath.c_str());

          // Simple matching, the node does not contain any resource parameters
          if (nodepath == resourceName) {
            resolvedResource.setMatchingNode(*node);
            HTTPS_LOGD("It's a match! Path: %s", nodepath.c_str());
            break;
          }
        } else {
          HTTPS_LOGD("Testing parameter match on %s", nodepath.c_str());

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
              HTTPS_LOGD("No match on static part %d", pIdx);
            } else {
              // static part did match, increase pointers
              nodeIdx += staticLength + 1; // +1 to take care of the '*' placeholder.
              urlIdx  += staticLength; // The pointer should now point to the begin of the static part

              // Second step: Grab the parameter value
              if (nodeIdx == nodepath.length()) {
                // Easy case: parse until end of string
                params->setUrlParameter(pIdx, urlDecode(resourceName.substr(urlIdx)));
              } else {
                // parse until first char after the placeholder
                char terminatorChar = nodepath[nodeIdx];
                size_t terminatorPosition = resourceName.find(terminatorChar, urlIdx);
                if (terminatorPosition != std::string::npos) {
                  // We actually found the terminator
                  size_t dynamicLength = terminatorPosition-urlIdx;
                  params->setUrlParameter(pIdx, urlDecode(resourceName.substr(urlIdx, dynamicLength)));
                  urlIdx = urlIdx + dynamicLength;
                } else {
                  // We did not find the terminator
                  didMatch = false;
                  HTTPS_LOGD("No match on dynamic part %d", pIdx);
                }
              }
            } // static part did match
          } // placeholder loop

          // If there is some final static part to process
          if (didMatch && nodeIdx < nodepath.length()) {
            size_t staticLength = nodepath.length()-nodeIdx;
            if (nodepath.substr(nodeIdx, staticLength).compare(url.substr(urlIdx, staticLength))!=0) {
              didMatch = false;
              HTTPS_LOGD("No match, final static part did not match");
            } else {
              urlIdx += staticLength;
              // If there is some string remaining in the url that did not match
              if (urlIdx < resourceName.length()) {
                didMatch = false;
                HTTPS_LOGD("No match, URL is longer than final static part");
              }
            }
          }

          // Every check worked, so the full url matches and the params are set
          if (didMatch) {
            resolvedResource.setMatchingNode(*node);
            HTTPS_LOGD("It's a match!");
            break;
          }

        } // static/dynamic url
      } // method check
    } // node type check
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

void ResourceResolver::addMiddleware(const HTTPSMiddlewareFunction * mwFunction) {
  _middleware.push_back(mwFunction);
}

void ResourceResolver::removeMiddleware(const HTTPSMiddlewareFunction * mwFunction) {
  _middleware.erase(std::remove(_middleware.begin(), _middleware.end(), mwFunction), _middleware.end());
}

const std::vector<HTTPSMiddlewareFunction*> ResourceResolver::getMiddleware() {
  return _middleware;
}

void ResourceResolver::setDefaultNode(HTTPNode * defaultNode) {
  _defaultNode = defaultNode;
}

}
