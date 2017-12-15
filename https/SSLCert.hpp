/*
 * SSLCert.hpp
 *
 *  Created on: Dec 6, 2017
 *      Author: frank
 */

#ifndef HTTPS_SSLCERT_HPP_
#define HTTPS_SSLCERT_HPP_

#include <Arduino.h>

namespace httpsserver {

class SSLCert {
public:
	SSLCert(unsigned char * certData, uint16_t certLength, unsigned char * pkData, uint16_t pkLength);
	virtual ~SSLCert();

	uint16_t getCertLength();
	uint16_t getPKLength();
	unsigned char * getCertData();
	unsigned char * getPKData();

private:
	uint16_t _certLength;
	unsigned char * _certData;
	uint16_t _pkLength;
	unsigned char * _pkData;

};

} /* namespace httpsserver */

#endif /* HTTPS_SSLCERT_HPP_ */
