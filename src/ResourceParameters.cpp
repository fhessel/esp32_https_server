#include "ResourceParameters.hpp"
#include "HTTPSServerConstants.hpp"

namespace httpsserver {

ResourceParameters::ResourceParameters() {

}

ResourceParameters::~ResourceParameters() {

}

/**
 * @brief Checks whether a specific HTTPS query parameter is set.
 * 
 * Query parameters are key-value pairs that are appended to the URI after a question mark.
 * 
 * If the key exists (either as a value-less parameter or with a value), the function returns true.
 * 
 * @param name The parameter to check
 * @return true iff the parameter exists
 */
bool ResourceParameters::isQueryParameterSet(std::string const &name) {
  for(auto queryParam = _queryParams.begin(); queryParam != _queryParams.end(); ++queryParam) {
    if ((*queryParam).first.compare(name)==0) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Returns an HTTP query parameter.
 * 
 * Query parameters are key-value pairs that are appended to the URI after a question mark.
 * 
 * The name parameter specifies the name of the query parameter to retrieve. If it is set,
 * the value is written to the value parameter and true is returned. If the parameter does
 * not exist, value is left unchanged and false is returned. If the parameter is used
 * without a value, an empty string is written to value and true is returned.
 * 
 * @param name The name of the parameter to retrieve. If the parameter exists multiple times,
 * the first occurence is used for the value. Use beginQueryParameters() to retrieve all values.
 * @param value The target to write the value to, if the parameter exists.
 * @return true iff the parameter exists and the corresponding value has been written.
 */
bool ResourceParameters::getQueryParameter(std::string const &name, std::string &value) {
  for(auto queryParam = _queryParams.begin(); queryParam != _queryParams.end(); ++queryParam) {
    if ((*queryParam).first.compare(name)==0) {
      value=(*queryParam).second;
      return true;
    }
  }
  return false;
}

/**
 * @brief Returns the number of query parameters.
 * 
 * Query parameters are key-value pairs that are appended to the URI after a question mark.
 * 
 * @param unique If true, return the number of unique keys (using the same key multiple times
 * is counted only once). False by default, as checking for uniqueness is not efficient.
 * @return Number of query parameters
 */
size_t ResourceParameters::getQueryParameterCount(bool unique) {
  if (!unique) {
    return _queryParams.size();
  }
  size_t count = 0;
  for(auto a = _queryParams.begin(); a != _queryParams.end(); ++a) {
    bool exists = false;
    for(auto b = _queryParams.begin(); !exists && b != a; ++b) {
      exists = (*a).first.compare((*b).first)==0;
    }
    count += exists ? 0 : 1;
  }
  return count;
}

/**
 * @brief Provides iterator access to the query parameters
 * 
 * Query parameters are key-value pairs that are appended to the URI after a question mark.
 * 
 * If you want just a specific parameter, have a look at getQueryParameter()
 * 
 * The iterator will yield pairs of std::string, of which the first value specifies the
 * query parameter key and the second value corresponds to the query parameters value.
 * If the entry is value-less, the second value will be the empty string.
 * 
 * If the same key is used multiple times in the query, the iterator will yield it multiple
 * times, once for each occurence with the specific value.
 * 
 * @return Iterator over std::pairs of std::strings that represent (key, value) pairs
 */
std::vector<std::pair<std::string,std::string>>::iterator ResourceParameters::beginQueryParameters() {
  return _queryParams.begin();
}

/**
 * @brief Counterpart to beginQueryParameters() for iterating over query parameters
 */
std::vector<std::pair<std::string,std::string>>::iterator ResourceParameters::endQueryParameters() {
  return _queryParams.end();
}

void ResourceParameters::setQueryParameter(std::string const &name, std::string const &value) {
  std::pair<std::string, std::string> param;
  param.first = name;
  param.second = value;
  _queryParams.push_back(param);
}

/**
 * @brief Checks for the existence of a path parameter and returns it as string.
 *
 * Path parameters are defined by an asterisk as placeholder when specifying the path of
 * the ResourceNode and addressed by an index starting at 0 for the first parameter.
 * 
 * For values of idx that have no matching placeholder, value is left unchanged and the
 * method will return false.
 * 
 * @param idx Defines the index of the parameter to return, starting with 0.
 * @param value The value is written into this parameter.
 * @return true iff the value could be written.
 */
bool ResourceParameters::getPathParameter(size_t const idx, std::string &value) {
  if (idx < _pathParams.size()) {
    value = _pathParams.at(idx);
    return true;
  }
  return false;
}

/**
 * @brief Directly returns a path parameter
 *
 * Path parameters are defined by an asterisk as placeholder when specifying the path of
 * the ResourceNode and addressed by an index starting at 0 for the first parameter.
 * 
 * This method will return the parameter specified by the index. The caller is responsible
 * to assure that the index exists. Otherwise, an empty string will be returned.
 * 
 * @param idx Defines the index of the parameter to return, starting with 0.
 * @return the value of the placeholder
 */
std::string ResourceParameters::getPathParameter(size_t const idx) {
  if (idx < _pathParams.size()) {
    return _pathParams.at(idx);
  }
  return "";
}

void ResourceParameters::resetPathParameters() {
  _pathParams.clear();
}

void ResourceParameters::setPathParameter(size_t idx, std::string const &val) {
  if(idx>=_pathParams.size()) {
    _pathParams.resize(idx + 1);
  }
  _pathParams.at(idx) = val;
}

} /* namespace httpsserver */
