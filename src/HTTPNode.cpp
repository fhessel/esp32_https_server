#include "HTTPNode.hpp"

namespace httpsserver {

  HTTPNode::HTTPNode(const std::string path, const HTTPNodeType nodeType, const std::string tag):
    _path(std::move(path)),
    _tag(std::move(tag)),
    _nodeType(nodeType) {

    // Count the parameters
    _urlParamCount = 0;
    size_t idx = 0;
    while((idx = path.find("/*", idx)) != std::string::npos) {
      _urlParamCount+=1;
      // If we don't do this, the same index will be found again... and again... and again...
      idx+=1;
    };

    // Check if there are parameters
    if (_urlParamCount > 0) {
      // If there are parameters, store their indices
      _urlParamIdx = new size_t[_urlParamCount];
      for(int i = 0; i < _urlParamCount; i++) {
        _urlParamIdx[i] = path.find("/*", i==0 ? 0 : _urlParamIdx[i-1])+1;
      }
    } else {
      _urlParamIdx = NULL;
    }
  }

  HTTPNode::~HTTPNode() {
    if (_urlParamIdx != NULL) {
      delete[] _urlParamIdx;
    }
  }

  bool HTTPNode::hasUrlParameter() {
    return _urlParamCount > 0;
  }

  size_t HTTPNode::getParamIdx(uint8_t idx) {
    if (idx<_urlParamCount) {
      return _urlParamIdx[idx];
    } else {
      return -1;
    }
  }

  uint8_t HTTPNode::getUrlParamCount() {
    return _urlParamCount;
  }
}