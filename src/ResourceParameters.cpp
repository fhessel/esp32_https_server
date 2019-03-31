#include "ResourceParameters.hpp"

namespace httpsserver {

ResourceParameters::ResourceParameters() {

}

ResourceParameters::~ResourceParameters() {

}

bool ResourceParameters::isRequestParameterSet(std::string const &name) {
  for(auto reqParam = _reqParams.begin(); reqParam != _reqParams.end(); ++reqParam) {
    if ((*reqParam).first.compare(name)==0) {
      return true;
    }
  }
  return false;
}

std::string ResourceParameters::getRequestParameter(std::string const &name) {
  for(auto reqParam = _reqParams.begin(); reqParam != _reqParams.end(); ++reqParam) {
    if ((*reqParam).first.compare(name)==0) {
      return (*reqParam).second;
    }
  }
  return "";
}

uint16_t ResourceParameters::getRequestParameterInt(std::string const &name) {
  return parseInt(getRequestParameter(name));
}

void ResourceParameters::setRequestParameter(std::string const &name, std::string const &value) {
  std::pair<std::string, std::string> param;
  param.first = name;
  param.second = value;
  _reqParams.push_back(param);
}

/**
 * Returns an URL parameter as string.
 *
 * URL parameters are defined by resource nodes like object/?/property
 *
 * The parameter idx defines the index of the parameter, starting with 0.
 */
std::string ResourceParameters::getUrlParameter(uint8_t idx) {
  return _urlParams.at(idx);
}

/**
 * Returns an URL parameter as int.
 *
 * URL parameters are defined by resource nodes like object/?/property
 *
 * The parameter idx defines the index of the parameter, starting with 0.
 */
uint16_t ResourceParameters::getUrlParameterInt(uint8_t idx) {
  return parseInt(getUrlParameter(idx));
}

void ResourceParameters::resetUrlParameters() {
  _urlParams.clear();
}

void ResourceParameters::setUrlParameter(uint8_t idx, std::string const &val) {
  if(idx>=_urlParams.capacity()) {
    _urlParams.resize(idx + 1);
  }
  _urlParams.at(idx) = val;
}

} /* namespace httpsserver */
