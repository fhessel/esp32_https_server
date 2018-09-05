#ifndef SRC_SSLCERT_HPP_
#define SRC_SSLCERT_HPP_

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

#endif /* SRC_SSLCERT_HPP_ */
