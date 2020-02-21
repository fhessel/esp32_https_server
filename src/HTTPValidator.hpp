#ifndef SRC_HTTPVALIDATOR_HPP_
#define SRC_HTTPVALIDATOR_HPP_

#include <string>

namespace httpsserver {

typedef bool (HTTPValidationFunction)(std::string);

/**
 * \brief Internal representation of a validator function
 */
class HTTPValidator {
public:
  HTTPValidator(const uint8_t idx, const HTTPValidationFunction * validatorFunction);
  virtual ~HTTPValidator();
  const uint8_t _idx;
  const HTTPValidationFunction * _validatorFunction;
};

} /* namespace httpsserver */

#endif /* SRC_HTTPVALIDATOR_HPP_ */
