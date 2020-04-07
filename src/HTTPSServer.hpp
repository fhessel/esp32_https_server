#ifndef SRC_HTTPSSERVER_HPP_
#define SRC_HTTPSSERVER_HPP_

// Standard library
#include <string>

// Arduino stuff
#include <Arduino.h>
#ifndef HTTPS_DISABLE_IPV4
#include <IPAddress.h>
#endif
#ifndef HTTPS_DISABLE_IPV6
#include <IPv6Address.h>
#endif

// Required for SSL
#include "openssl/ssl.h"
#undef read

// Internal includes
#include "HTTPServer.hpp"
#include "HTTPSServerConstants.hpp"
#include "HTTPHeaders.hpp"
#include "HTTPHeader.hpp"
#include "ResourceNode.hpp"
#include "ResourceResolver.hpp"
#include "ResolvedResource.hpp"
#include "HTTPSConnection.hpp"
#include "SSLCert.hpp"

namespace httpsserver {

/**
 * \brief Main implementation of the HTTP Server with TLS support. Use HTTPServer for plain HTTP
 */
class HTTPSServer : public HTTPServer {
public:
#ifndef HTTPS_DISABLE_IPV4
  /**
   * \brief Create a server instance that binds to an IPv4 address
   * 
   * \param cert A reference to an SSLCert to use with the server. Must be valid during the server's lifetime
   * \param bindAddress IPAddress to bind to. Use IPAddress() to bind to all IPv4 interfaces.
   * \param port TCP port to run the server on. Defaults to 443 (HTTPS default)
   * \param maxConnections Maximum number of parallel connections handled by the server. Defaults to 4 (more might cause trouble on ESP32s with low memory)
   */
  HTTPSServer(SSLCert * cert, const IPAddress bindAddress = IPAddress(),
    const uint16_t portHTTPS = 443, const uint8_t maxConnections = 4);
#endif
#ifndef HTTPS_DISABLE_IPV6
#ifndef HTTPS_DISABLE_IPV4
  /**
   * \brief Create a server instance that binds to an IPv6 address
   * 
   * \param cert A reference to an SSLCert to use with the server. Must be valid during the server's lifetime
   * \param bindAddress IPAddress to bind to. Use IPv6Address() to bind to all IPv6 interfaces.
   * \param port TCP port to run the server on. Defaults to 443 (HTTPS default)
   * \param maxConnections Maximum number of parallel connections handled by the server. Defaults to 4 (more might cause trouble on ESP32s with low memory)
   */
  HTTPSServer(SSLCert * cert, const IPv6Address bindAddress = IPv6Address(),
    const uint16_t portHTTPS = 443, const uint8_t maxConnections = 4);
#else
  HTTPSServer(SSLCert * cert, const IPv6Address bindAddress,
    const uint16_t portHTTPS = 443, const uint8_t maxConnections = 4);
#endif
#endif
  virtual ~HTTPSServer();

private:
  // Static configuration. Port, keys, etc. ====================
  // Certificate that should be used (includes private key)
  SSLCert * _cert;
 
  //// Runtime data ============================================
  SSL_CTX * _sslctx;
  // Status of the server: Are we running, or not?

  // Setup functions
  virtual uint8_t setupSocket();
  virtual void teardownSocket();
  uint8_t setupSSLCTX();
  uint8_t setupCert();

  // Helper functions
  virtual int createConnection(int idx);
};

} /* namespace httpsserver */

#endif /* SRC_HTTPSSERVER_HPP_ */
