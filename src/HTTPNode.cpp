#include "HTTPNode.hpp"

namespace httpsserver {

  HTTPNode::HTTPNode(std::string const &path, const HTTPNodeType nodeType, std::string const &tag):
    _path(path),
    _tag(tag),
    _nodeType(nodeType) {

    // Create vector for valdiators
    _validators = new std::vector<HTTPValidator*>();

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

    // Delete validator references
    for(std::vector<HTTPValidator*>::iterator validator = _validators->begin(); validator != _validators->end(); ++validator) {
      delete *validator;
    }
    delete _validators;
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

  void HTTPNode::addURLParamValidator(uint8_t paramIdx, const HTTPValidationFunction * validator) {
    _validators->push_back(new HTTPValidator(paramIdx, validator));
  }

  std::vector<HTTPValidator*> * HTTPNode::getValidators() {
    return _validators;
  }
}
