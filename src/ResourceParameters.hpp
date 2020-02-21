#ifndef SRC_RESOURCEPARAMETERS_HPP_
#define SRC_RESOURCEPARAMETERS_HPP_

#include <Arduino.h>

#include <string>
// Arduino declares it's own min max, incompatible with the stl...
#undef min
#undef max
#include <vector>
#include <utility>

#include "util.hpp"

namespace httpsserver {

class ResourceResolver;

/**
 * @brief The ResourceParameters provide access to the parameters passed in the URI.
 * 
 * There are two types of parameters: Path parameters and query parameters.
 * 
 * Path parameters are the values that fill the asterisk placeholders in the route
 * definition of a ResourceNode.
 * 
 * Query parameters are the key-value pairs after a question mark which can be added
 * to each request, either by specifying them manually or as result of submitting an
 * HTML form with a GET as method property.
 */
class ResourceParameters {
public:
  ResourceParameters();
  virtual ~ResourceParameters();

  bool isQueryParameterSet(std::string const &name);
  bool getQueryParameter(std::string const &name, std::string &value);
  std::vector<std::pair<std::string,std::string>>::iterator beginQueryParameters();
  std::vector<std::pair<std::string,std::string>>::iterator endQueryParameters();
  size_t getQueryParameterCount(bool unique=false);
  bool getPathParameter(size_t const idx, std::string &value);
  std::string getPathParameter(size_t const idx);

protected:
  friend class ResourceResolver;
  void setQueryParameter(std::string const &name, std::string const &value);
  void resetPathParameters();
  void setPathParameter(size_t idx, std::string const &val);

private:
  /** Parameters in the path of the URL, the actual values for asterisk placeholders */
  std::vector<std::string> _pathParams;
  /** HTTP Query parameters, as key-value pairs */
  std::vector<std::pair<std::string, std::string>> _queryParams;
};

} /* namespace httpsserver */

#endif /* SRC_RESOURCEPARAMETERS_HPP_ */
