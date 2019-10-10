#include "TLSTickets.hpp"
#include "HTTPSServerConstants.hpp"

#include "mbedtls/net_sockets.h"

// Low level SSL implementation on ESP32
// Copied from esp-idf/components/openssl/platform/ssl_pm.c 
struct ssl_pm {    
  mbedtls_net_context fd;
  mbedtls_net_context cl_fd;
  mbedtls_ssl_config conf; 
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_ssl_context ssl;
  mbedtls_entropy_context entropy;
};

namespace httpsserver {

int TLSTickets::hardware_random(void * p_rng, unsigned char * output, size_t output_len) {
   esp_fill_random(output, output_len);
   return 0;
}

TLSTickets::TLSTickets(const char* tag, uint32_t lifetimeSeconds, bool useHWRNG) {
  _initOk = false;
  _useHWRNG = useHWRNG;

  // Setup TLS tickets context
  int ret = -1;
  if (_useHWRNG) {
    mbedtls_ssl_ticket_init(&_ticketCtx);
    ret = mbedtls_ssl_ticket_setup(
      &_ticketCtx,
      TLSTickets::hardware_random,
      NULL,
      MBEDTLS_CIPHER_AES_256_GCM, 
      lifetimeSeconds
    );
  } else {
		mbedtls_entropy_init(&_entropy); 			
    mbedtls_ctr_drbg_init(&_ctr_drbg);
    mbedtls_ssl_ticket_init(&_ticketCtx);
    ret = mbedtls_ctr_drbg_seed(
      &_ctr_drbg, 
      mbedtls_entropy_func, 
      &_entropy, 
      (unsigned char*)tag, 
      strlen(tag)
    );
    if (ret == 0) {
      ret = mbedtls_ssl_ticket_setup(
        &_ticketCtx,
        mbedtls_ctr_drbg_random,
        &_ctr_drbg,
        MBEDTLS_CIPHER_AES_256_GCM, 
        lifetimeSeconds
      );    
    }
  }
  if (ret != 0) return;

  _initOk = true;
  HTTPS_LOGI("Using TLS session tickets");
}

TLSTickets::~TLSTickets() {
  if (!_useHWRNG) {
    mbedtls_ctr_drbg_free(&_ctr_drbg);
    mbedtls_entropy_free(&_entropy);
  }
  mbedtls_ssl_ticket_free(&_ticketCtx);
}
  
bool TLSTickets::enable(SSL * ssl) {
  bool res = false;
  if (_initOk && ssl && ssl->ssl_pm) {      
    // Get handle of low-level mbedtls structures for the session
    struct ssl_pm * ssl_pm = (struct ssl_pm *) ssl->ssl_pm;
    // Configure TLS ticket callbacks using default MbedTLS implementation
    mbedtls_ssl_conf_session_tickets_cb(
      &ssl_pm->conf,
      mbedtls_ssl_ticket_write,
      mbedtls_ssl_ticket_parse,
      &_ticketCtx
    );        
    res = true;
  }
  return res;
}
    
} /* namespace httpsserver */