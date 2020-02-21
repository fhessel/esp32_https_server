#include "HTTPNode.hpp"
#include "HTTPSServerConstants.hpp"

namespace httpsserver {

  HTTPNode::HTTPNode(std::string const &path, const HTTPNodeType nodeType, std::string const &tag):
    _path(path),
    _tag(tag),
    _nodeType(nodeType) {

    // Count the parameters and store the indices
    size_t idx = 0;
    while((idx = path.find("/*", idx)) != std::string::npos) {
      // Skip the "/*"
      idx+=2;
      // Assure that we have either /* at the URL's end or /*/ somewhere in between
      if (idx == path.size() || path[idx] == '/') {
        _pathParamIdx.push_back(idx - 1);
      }
    };
  }
  
  HTTPNode::~HTTPNode() {
    // Delete validator references
    for(std::vector<HTTPValidator*>::iterator validator = _validators.begin(); validator != _validators.end(); ++validator) {
      delete *validator;
    }
  }

  bool HTTPNode::hasPathParameter() {
    return !_pathParamIdx.empty();
  }

  ssize_t HTTPNode::getParamIdx(size_t idx) {
    if (idx<_pathParamIdx.size()) {
      return _pathParamIdx[idx];
    } else {
      return -1;
    }
  }

  size_t HTTPNode::getPathParamCount() {
    return _pathParamIdx.size();
  }

  void HTTPNode::addPathParamValidator(size_t paramIdx, const HTTPValidationFunction * validator) {
    _validators.push_back(new HTTPValidator(paramIdx, validator));

  }

  std::vector<HTTPValidator*> * HTTPNode::getValidators() {
    return &_validators;
  }
}
