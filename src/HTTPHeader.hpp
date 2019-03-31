#ifndef SRC_HTTPHEADER_HPP_
#define SRC_HTTPHEADER_HPP_
#include <Arduino.h>

#include <string>

namespace httpsserver {

class HTTPHeader {
public:
  HTTPHeader(const std::string &name, const std::string &value);
  virtual ~HTTPHeader();
  const std::string _name;
  const std::string _value;
  std::string print();
};

} /* namespace httpsserver */

#endif /* SRC_HTTPHEADER_HPP_ */
