#include "HTTPValidator.hpp"

namespace httpsserver {

HTTPValidator::HTTPValidator(const uint8_t idx, const HTTPValidationFunction * validatorFunction):
  _idx(idx),
  _validatorFunction(validatorFunction) {

}

HTTPValidator::~HTTPValidator() {

}

} /* namespace httpsserver */
