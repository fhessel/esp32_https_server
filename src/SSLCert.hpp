#ifndef SRC_SSLCERT_HPP_
#define SRC_SSLCERT_HPP_

#include <Arduino.h>

#ifndef HTTPS_DISABLE_SELFSIGNING
#include <string>
#include <mbedtls/rsa.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/pk.h>
#include <mbedtls/x509.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/x509_csr.h>

#define HTTPS_SERVER_ERROR_KEYGEN 0x0F
#define HTTPS_SERVER_ERROR_KEYGEN_RNG 0x02
#define HTTPS_SERVER_ERROR_KEYGEN_SETUP_PK 0x03
#define HTTPS_SERVER_ERROR_KEYGEN_GEN_PK 0x04
#define HTTPS_SERVER_ERROR_KEY_WRITE_PK 0x05
#define HTTPS_SERVER_ERROR_KEY_OUT_OF_MEM 0x06
#define HTTPS_SERVER_ERROR_CERTGEN 0x1F
#define HTTPS_SERVER_ERROR_CERTGEN_RNG 0x12
#define HTTPS_SERVER_ERROR_CERTGEN_READKEY 0x13
#define HTTPS_SERVER_ERROR_CERTGEN_WRITE 0x15
#define HTTPS_SERVER_ERROR_CERTGEN_OUT_OF_MEM 0x16
#define HTTPS_SERVER_ERROR_CERTGEN_NAME 0x17
#define HTTPS_SERVER_ERROR_CERTGEN_SERIAL 0x18
#define HTTPS_SERVER_ERROR_CERTGEN_VALIDITY 0x19

#endif // !HTTPS_DISABLE_SELFSIGNING

namespace httpsserver {

class SSLCert {
public:
  SSLCert(
    unsigned char * certData = NULL,
    uint16_t certLength = 0,
    unsigned char * pkData = NULL,
    uint16_t pkLength = 0
  );
  virtual ~SSLCert();

  uint16_t getCertLength();
  uint16_t getPKLength();
  unsigned char * getCertData();
  unsigned char * getPKData();

  void setPK(unsigned char * _pkData, uint16_t length);
  void setCert(unsigned char * _certData, uint16_t length);

  /** Clears the key buffers and delets them. */
  void clear();

private:
  uint16_t _certLength;
  unsigned char * _certData;
  uint16_t _pkLength;
  unsigned char * _pkData;

};

#ifndef HTTPS_DISABLE_SELFSIGNING

enum SSLKeySize {
  KEYSIZE_1024 = 1024,
  KEYSIZE_2048 = 2048,
  KEYSIZE_4096 = 4096
};

/**
 * This function creates a new self-signed certificate for the given hostname on the heap.
 * Make sure to clear() it before you delete it.
 * 
 * The distinguished name (dn) parameter has to follow the x509 specifications. An example
 * would be:
 *   CN=myesp.local,O=acme,C=US
 * 
 * The strings validFrom and validUntil have to be formatted like this:
 * "20190101000000", "20300101000000"
 * 
 * This will take some time, so you should probably write the certificate data to non-volatile
 * storage when you are done.
 */
int createSelfSignedCert(SSLCert &certCtx, SSLKeySize keySize, std::string dn, std::string validFrom = "20190101000000", std::string validUntil = "20300101000000");

#endif // !HTTPS_DISABLE_SELFSIGNING

} /* namespace httpsserver */

#endif /* SRC_SSLCERT_HPP_ */
