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

struct requestparam_t {std::string name; std::string value;};

/**
 * \brief Class used to handle access to the URL parameters
 */
class ResourceParameters {
public:
  ResourceParameters();
  virtual ~ResourceParameters();

  bool isRequestParameterSet(std::string const &name);
  std::string getRequestParameter(std::string const &name);
  uint16_t getRequestParameterInt(std::string const &name);
  void setRequestParameter(std::string const &name, std::string const &value);

  std::string getUrlParameter(size_t idx);
  uint16_t getUrlParameterInt(size_t idx);
  void resetUrlParameters();
  void setUrlParameter(size_t idx, std::string const &val);

private:
  std::vector<std::string> _urlParams;
  std::vector<std::pair<std::string, std::string>> _reqParams;
};

} /* namespace httpsserver */

#endif /* SRC_RESOURCEPARAMETERS_HPP_ */
