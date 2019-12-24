#include "HTTPNode.hpp"

namespace httpsserver {

  HTTPNode::HTTPNode(std::string const &path, const HTTPNodeType nodeType, std::string const &tag):
    _path(path),
    _tag(tag),
    _nodeType(nodeType) {

    // Create vector for valdiators
    _validators = new std::vector<HTTPValidator*>();

    // Count the parameters
    _pathParamCount = 0;
    size_t idx = 0;
    while((idx = path.find("/*", idx)) != std::string::npos) {
      _pathParamCount+=1;
      // If we don't do this, the same index will be found again... and again... and again...
      idx+=1;
    };

    // Check if there are parameters
    if (_pathParamCount > 0) {
      // If there are parameters, store their indices
      _pathParamIdx = new size_t[_pathParamCount];
      for(int i = 0; i < _pathParamCount; i++) {
        _pathParamIdx[i] = path.find("/*", i==0 ? 0 : _pathParamIdx[i-1])+1;
      }
    } else {
      _pathParamIdx = NULL;
    }
  }

  HTTPNode::~HTTPNode() {
    if (_pathParamIdx != NULL) {
      delete[] _pathParamIdx;
    }

    // Delete validator references
    for(std::vector<HTTPValidator*>::iterator validator = _validators->begin(); validator != _validators->end(); ++validator) {
      delete *validator;
    }
    delete _validators;
  }

  bool HTTPNode::hasPathParameter() {
    return _pathParamCount > 0;
  }

  size_t HTTPNode::getParamIdx(uint8_t idx) {
    if (idx<_pathParamCount) {
      return _pathParamIdx[idx];
    } else {
      return -1;
    }
  }

  size_t HTTPNode::getPathParamCount() {
    return _pathParamCount;
  }

  void HTTPNode::addPathParamValidator(size_t paramIdx, const HTTPValidationFunction * validator) {
    _validators->push_back(new HTTPValidator(paramIdx, validator));
  }

  std::vector<HTTPValidator*> * HTTPNode::getValidators() {
    return _validators;
  }
}
