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

// FreeRTOS 
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Internal includes
#include "HTTPSServerConstants.hpp"
#include "HTTPHeaders.hpp"
#include "HTTPHeader.hpp"
#include "ResourceNode.hpp"
#include "ResourceResolver.hpp"
#include "ResolvedResource.hpp"
#include "HTTPConnection.hpp"
#include "HTTPWorker.hpp"

namespace httpsserver {

/**
 * \brief Main implementation for the plain HTTP server. Use HTTPSServer for TLS support
 */
class HTTPServer : public ResourceResolver {
public:
  HTTPServer(const uint16_t portHTTPS = 80, const uint8_t maxConnections = 8, const in_addr_t bindAddress = 0);
  virtual ~HTTPServer();

  uint8_t start();
  void stop();
  bool isRunning();

  // Return value is remaining miliseconds if function returned early
  int loop(int timeoutMs = 0);

  void setDefaultHeader(std::string name, std::string value);

  // Enable separate FreeRTOS tasks handling for connections. 
  // Must be called before start()
  void enableWorkers(
    uint8_t numWorkers = 2, 
    size_t taskStackSize = HTTPS_CONN_TASK_STACK_SIZE, 
    int taskPriority = HTTPS_CONN_TASK_PRIORITY
  );

  HTTPHeaders * getDefaultHeaders();
  int serverSocket();

protected:
  friend class HTTPWorker;
  // Static configuration. Port, keys, etc. ====================
  // Certificate that should be used (includes private key)
  const uint16_t _port;

  // Max parallel connections that the server will accept
  const uint8_t _maxConnections;
  // Address to bind to (0 = all interfaces)
  const in_addr_t _bindAddress;

  //// Runtime data ============================================
  // The array of connections that are currently active
  HTTPConnection ** _connections;
  // Status of the server: Are we running, or not?
  boolean _running;
  // The server socket
  int _socket;
  
  // Keep state if we have pendig connections
  HTTPConnection * _pendingConnection = NULL;
  bool _pendingData = false;
  bool _lookForIdleConnection = false;
  
  // HTTPWorker(s) and syncronization
  uint8_t _numWorkers = 0;
  SemaphoreHandle_t _selectMutex = NULL;
  SemaphoreHandle_t * _connectionMutex = NULL;
  QueueHandle_t _workQueue = NULL;
  HTTPWorker ** _workers;

  // The server socket address, that our service is bound to
  sockaddr_in _sock_addr;
  // Headers that are included in every response
  HTTPHeaders _defaultHeaders;

  // Setup functions
  virtual uint8_t setupSocket();
  virtual void teardownSocket();

  // Internal functions
  void manageConnections(int maxTimeoutMs);
  bool doQueuedWork(TickType_t waitDelay);

  // Helper functions
  virtual HTTPConnection * createConnection();
  //int createConnection(int idx);

};

}

#endif /* SRC_HTTPSERVER_HPP_ */
