/*
 * ResourceParameters.cpp
 *
 *  Created on: Dec 18, 2017
 *      Author: frank
 */

#include "ResourceParameters.hpp"

namespace httpsserver {

ResourceParameters::ResourceParameters() {

}

ResourceParameters::~ResourceParameters() {

}


std::string ResourceParameters::getRequestParameter(std::string &name) {
	return "";
}

uint16_t ResourceParameters::getRequestParameterInt(std::string &name) {
	return parseInt(getRequestParameter(name));
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

void ResourceParameters::setUrlParameter(uint8_t idx, std::string val) {
	if(idx>=_urlParams.capacity()) {
		_urlParams.resize(idx + 1);
	}
	_urlParams.at(idx) = val;
}

} /* namespace httpsserver */
