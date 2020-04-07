#ifndef SRC_HTTPSERVER_HPP_
#define SRC_HTTPSERVER_HPP_

// Standard library
#include <string>

// Arduino stuff
#include <Arduino.h>
#ifndef HTTPS_DISABLE_IPV4
#include <IPAddress.h>
#ifdef HTTPS_DISABLE_IPV4
#error You cannot HTTPS_DISABLE_IPV4 and HTTPS_DISABLE_IPV6 at the same time
#endif
#endif
#ifndef HTTPS_DISABLE_IPV6
#include <IPv6Address.h>
#ifdef HTTPS_DISABLE_IPV4
#error You cannot HTTPS_DISABLE_IPV4 and HTTPS_DISABLE_IPV6 at the same time
#endif
#endif

// Required for sockets
#include "lwip/netdb.h"
#undef read
#include "lwip/sockets.h"
#include "lwip/inet.h"

// Internal includes
#include "HTTPSServerConstants.hpp"
#include "HTTPHeaders.hpp"
#include "HTTPHeader.hpp"
#include "ResourceNode.hpp"
#include "ResourceResolver.hpp"
#include "ResolvedResource.hpp"
#include "HTTPConnection.hpp"

namespace httpsserver {

/**
 * \brief Main implementation for the plain HTTP server. Use HTTPSServer for TLS support
 */
class HTTPServer : public ResourceResolver {
public:
#ifndef HTTPS_DISABLE_IPV4
  /**
   * \brief Create a server instance that binds to an IPv4 address
   * 
   * \param bindAddress IPAddress to bind to. Use IPAddress() to bind to all IPv4 interfaces.
   * \param port TCP port to run the server on. Defaults to 80 (HTTP default)
   * \param maxConnections Maximum number of parallel connections handled by the server. Defaults to 8.
   */
  HTTPServer(const IPAddress bindAddress = IPAddress(), const uint16_t port = 80, const uint8_t maxConnections = 8);
#endif
#ifndef HTTPS_DISABLE_IPV6
#ifdef HTTPS_DISABLE_IPV4
  HTTPServer(const IPv6Address bindAddress, const uint16_t port = 80, const uint8_t maxConnections = 8);
#else
  /**
   * \brief Create a server instance that binds to an IPv6 address
   * 
   * \param bindAddress IPv6Address to bind to. Use IPv6Address() to bind to all IPv6 interfaces.
   * \param port TCP port to run the server on. Defaults to 80 (HTTP default)
   * \param maxConnections Maximum number of parallel connections handled by the server. Defaults to 8.
   */
  HTTPServer(const IPv6Address bindAddress = IPv6Address(), const uint16_t port = 80, const uint8_t maxConnections = 8);
#endif
#endif
  virtual ~HTTPServer();

  uint8_t start();
  void stop();
  bool isRunning();

  void loop();

  void setDefaultHeader(std::string name, std::string value);

protected:
  // Static configuration. Port, keys, etc. ====================
  // Certificate that should be used (includes private key)
  const uint16_t _port;

  // Max parallel connections that the server will accept
  const uint8_t _maxConnections;
#ifndef HTTPS_DISABLE_IPV4
  /**
   * \brief IPv4 addresses to bind to (0 = all IPv4 interfaces)
   * 
   * To enable IPv4, add at least one element to this vector.
   * The content cannot be changed while the server is _running
   */
  std::vector<in_addr_t> _bindAddressesV4;
#endif
#ifndef HTTPS_DISABLE_IPV6
  /**
   * \brief IPv6 addresses to bind to (0 = all IPv6 interfaces)
   * 
   * To enable IPv6, add at least one element to this vector.
   * The content cannot be changed while the server is _running
   */
  std::vector<in6_addr> _bindAddressesV6;
#endif

  //// Runtime data ============================================
  // The array of connections that are currently active
  HTTPConnection ** _connections;
  // Status of the server: Are we running, or not?
  boolean _running;
  // The server socket
  int _socket;

  // The server socket address, that our service is bound to
  sockaddr _sock_addr;
  // Headers that are included in every response
  HTTPHeaders _defaultHeaders;

  // Setup functions
  virtual uint8_t setupSocket();
  virtual void teardownSocket();

  // Helper functions
  virtual int createConnection(int idx);
};

}

#endif /* SRC_HTTPSERVER_HPP_ */
