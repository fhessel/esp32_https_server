#ifndef SRC_HTTPSERVER_HPP_
#define SRC_HTTPSERVER_HPP_

// Standard library
#include <string>

// Arduino stuff
#include <Arduino.h>

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
  HTTPServer(const uint16_t portHTTPS = 80, const uint8_t maxConnections = 8, const in_addr_t bindAddress = INADDR_ANY);
  HTTPServer(const uint16_t portHTTPS, const uint8_t maxConnections, const uint8_t bindAddress[16]);
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
  // Address to bind to (INADDR_ANY or IN6ADDR_ANY_INIT = all interfaces)
  union bindAddressv4v6 {
    struct in_addr ipv4bindAddress;
    struct in6_addr ipv6bindAddress;
  };
  bindAddressv4v6 _bindAddress;
  // Internal flag for IPv6 support
  const bool _useIPv6;

  //// Runtime data ============================================
  // The array of connections that are currently active
  HTTPConnection ** _connections;
  // Status of the server: Are we running, or not?
  boolean _running;
  // The server socket
  int _socket;

  // The server socket address, that our service is bound to
  sockaddr_storage _sock_addr;
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
