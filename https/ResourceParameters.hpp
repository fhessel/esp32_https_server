/*
 * ResourceParameters.hpp
 *
 *  Created on: Dec 18, 2017
 *      Author: frank
 */

#ifndef HTTPS_RESOURCEPARAMETERS_HPP_
#define HTTPS_RESOURCEPARAMETERS_HPP_

#include <Arduino.h>

#include <string>
// Arduino declares it's own min max, incompatible with the stl...
#undef min
#undef max
#include <vector>

#include "util.hpp"

namespace httpsserver {

class ResourceParameters {
public:
	ResourceParameters();
	virtual ~ResourceParameters();

	std::string getRequestParameter(std::string &name);
	uint16_t getRequestParameterInt(std::string &name);

	std::string getUrlParameter(uint8_t idx);
	uint16_t getUrlParameterInt(uint8_t idx);
	void resetUrlParameters();
	void setUrlParameterCount(uint8_t idx);
	void setUrlParameter(uint8_t idx, std::string val);

private:
	std::vector<std::string> _urlParams;
};

} /* namespace httpsserver */

#endif /* HTTPS_RESOURCEPARAMETERS_HPP_ */
