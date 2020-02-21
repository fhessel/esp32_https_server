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
  // Store this index to stop path parsing there
  size_t pathEnd = reqparamIdx != std::string::npos ? reqparamIdx : url.size();

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
      std::string param = nextparamIdx == std::string::npos ?
         url.substr(reqparamIdx) :
         url.substr(reqparamIdx, nextparamIdx - reqparamIdx);

      if (param.length() > 0) {
        // Find the position where the string has to be split
        size_t nvSplitIdx = param.find('=');

        // Use empty string if only name is set. /foo?bar&baz=1 will return "" for bar
        std::string name  = urlDecode(param.substr(0, nvSplitIdx));
        std::string value = "";
        if (nvSplitIdx != std::string::npos) {
          value = urlDecode(param.substr(nvSplitIdx+1));
        }

        // Now we finally have name and value.
        params->setQueryParameter(name, value);
      }

      // Update reqparamIdx
      reqparamIdx = nextparamIdx;

    } while(reqparamIdx != std::string::npos);
  }


  // Check whether a resource matches

  for(std::vector<HTTPNode*>::iterator itNode = _nodes->begin(); itNode != _nodes->end(); ++itNode) {
    params->resetPathParameters();
    HTTPNode *node = *itNode;
    if (node->_nodeType==nodeType) {
      if (
        // For handler functions, check the method declared with the node
        (node->_nodeType==HANDLER_CALLBACK && ((ResourceNode*)node)->_method == method) ||
        // For websockets, the specification says that GET is the only choice
        (node->_nodeType==WEBSOCKET && method=="GET")
      ) {
        HTTPS_LOGD("Testing route %s", node->_path.c_str());
        bool match = true;
        size_t paramCount = node->getPathParamCount();
        // indices in input and pattern
        size_t inputIdx = 0, pathIdx = 0;
        HTTPS_LOGD("(INIT) inputIdx: %d, pathIdx: %d, pathEnd: %d, path: %s, url: %s",
          inputIdx, pathIdx, pathEnd, node->_path.c_str(), url.c_str());

        for (size_t paramIdx = 0; match && paramIdx < paramCount; paramIdx += 1) {
          HTTPS_LOGD("(LOOP) inputIdx: %d, pathIdx: %d, pathEnd: %d, path: %s, url: %s",
            inputIdx, pathIdx, pathEnd, node->_path.c_str(), url.c_str());
          // Test static path before the parameter
          size_t paramPos = node->getParamIdx(paramIdx);
          size_t staticLength = paramPos - pathIdx;
          match &= url.substr(inputIdx, staticLength) == node->_path.substr(pathIdx, staticLength);
          inputIdx += staticLength;
          pathIdx += staticLength;

          // Extract parameter value
          if (match) {
            size_t paramEnd = url.find('/', inputIdx);
            if (paramEnd == std::string::npos && inputIdx <= pathEnd) {
              // Consume the remaining input (might be "" for the last param)
              paramEnd = pathEnd;
            }
            if (paramEnd != std::string::npos) {
              size_t paramLength = paramEnd - inputIdx;
              params->setPathParameter(paramIdx, urlDecode(url.substr(inputIdx, paramLength)));
              pathIdx += 1;
              inputIdx += paramLength;
            } else {
              match = false;
              HTTPS_LOGD("(LOOP) No match on param part");
            }
          } else {
            HTTPS_LOGD("(LOOP) No match on static part");
          }
        }
        HTTPS_LOGD("(STTC) inputIdx: %d, pathIdx: %d, pathEnd: %d, path: %s, url: %s",
          inputIdx, pathIdx, pathEnd, node->_path.c_str(), url.c_str());
        // Test static path after the parameter (up to pathEnd)
        if (match) {
          match = url.substr(inputIdx, pathEnd - inputIdx)==node->_path.substr(pathIdx);
        }
        HTTPS_LOGD("(END ) inputIdx: %d, pathIdx: %d, pathEnd: %d, path: %s, url: %s",
          inputIdx, pathIdx, pathEnd, node->_path.c_str(), url.c_str());

        if (match) {
          resolvedResource.setMatchingNode(node);
          HTTPS_LOGD("It's a match!");
          break;
        }
      } // method check
    } // node type check
  } // resource node for loop

  // If the resource did not match, configure the default resource
  if (!resolvedResource.didMatch() && _defaultNode != NULL) {
    params->resetPathParameters();
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
