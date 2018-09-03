/*
 * SSLCert.cpp
 *
 *  Created on: Dec 6, 2017
 *      Author: frank
 */

#include "../src/SSLCert.hpp"

namespace httpsserver {

SSLCert::SSLCert(unsigned char * certData, uint16_t certLength, unsigned char * pkData, uint16_t pkLength):
	_certLength(certLength),
	_certData(certData),
	_pkLength(pkLength),
	_pkData(pkData) {

}

SSLCert::~SSLCert() {
	// TODO Auto-generated destructor stub
}


uint16_t SSLCert::getCertLength() {
	return _certLength;
}

uint16_t SSLCert::getPKLength() {
	return _pkLength;
}

unsigned char * SSLCert::getCertData() {
	return _certData;
}

unsigned char * SSLCert::getPKData() {
	return _pkData;
}

} /* namespace httpsserver */
