#include "HTTPServer.hpp"

namespace httpsserver {


HTTPServer::HTTPServer(const uint16_t port, const uint8_t maxConnections, const in_addr_t bindAddress):
  _port(port),
  _maxConnections(maxConnections),
  _bindAddress(bindAddress) {

  _connections = NULL;
  _workers = NULL;

  // Configure runtime data
  _socket = -1;
  _selectMutex = xSemaphoreCreateMutex();
  _numWorkers = 0;
  _workQueue = NULL;
  _pendingConnection = NULL;
  _running = false;
}

HTTPServer::~HTTPServer() {

  // Stop the server.
  // This will also remove all existing connections
  if(_running) {
    stop();
  }

  // Delete allocated memory
  if (_workers) delete[] _workers;
  if (_connections) delete[] _connections;
  if (_connectionMutex) delete[] _connectionMutex;
  if (_selectMutex) vSemaphoreDelete(_selectMutex);
  if (_workQueue) vQueueDelete(_workQueue);
}

/**
 *  Enables workers, each running in separate FreeRTOS task
 */
void HTTPServer::enableWorkers(uint8_t numWorkers, size_t stackSize, int priority) {
  if (!_running && (numWorkers > 0)) {
    _numWorkers = numWorkers;
    if (_workers == NULL) {
      _workers = new HTTPWorker * [numWorkers];
      HTTPS_LOGD("Creating %d worker(s) (%u,%d)", numWorkers, stackSize, priority);
      for(uint8_t i = 0; i < numWorkers; i++) {
        _workers[i] = new HTTPWorker(this, stackSize, priority);
      }
    }
  }
}

/**
 * This method starts the server and begins to listen on the port
 */
uint8_t HTTPServer::start() {
  if (!_running) {
    if (setupSocket()) {
      // Create space for the connections if not using worker tasks
      if (!_workQueue) {
        _workQueue = xQueueCreate(2 * _maxConnections, sizeof(int8_t));
      }
      if (!_connections) {
        _connections = new HTTPConnection*[_maxConnections];
        for(uint8_t i = 0; i < _maxConnections; i++) _connections[i] = NULL;
      }
      if (!_connectionMutex) {
        _connectionMutex = new SemaphoreHandle_t[_maxConnections];
        for(uint8_t i = 0; i < _maxConnections; i++) _connectionMutex[i] = xSemaphoreCreateMutex();
      }
      _running = true;
      // start the workers
      if (_numWorkers > 0) {
        for(uint8_t i = 0; i < _numWorkers; i++) _workers[i]->start();
      }
      return 1;
    }
    return 0;
  } else {
    return 1;
  }
}

bool HTTPServer::isRunning() {
  return _running;
}

/**
 * This method stops the server
 */
void HTTPServer::stop() {

  if (_running) {
    // Set the flag that the server is stopped
    _running = false;
    xSemaphoreTake(_selectMutex, portMAX_DELAY); // We won't be releasing this

    if (_connections) {
      // Clean up the connections    
      bool hasOpenConnections = true;
      while(hasOpenConnections) {
        hasOpenConnections = false;
        for(int i = 0; i < _maxConnections; i++) {
          xSemaphoreTake(_connectionMutex[i], portMAX_DELAY);
          if (_connections[i] != NULL) {
            _connections[i]->closeConnection();

            // Check if closing succeeded. If not, we need to call the close function multiple times
            // and wait for the client
            if (_connections[i]->isClosed()) {
              delete _connections[i];
              _connections[i] = NULL;
            } else {
              hasOpenConnections = true;
            }
          }
          xSemaphoreGive(_connectionMutex[i]);
          vSemaphoreDelete(_connectionMutex[i]);
        }
        delay(1);
      }
    } // if (_connections)

    teardownSocket();

    // Server _running is false, workers should terminate themselves...
    if (_workers) {
      // Just give them invalid connection number if they are blocked on the work queue
      int8_t noWork = -1;
      for(int i = 0; i < _numWorkers; i++) xQueueSend(_workQueue, &noWork, 0);
      bool workerStillActive = false;
      do {
        for(int i = 0; i < _maxConnections; i++) {          
          if (_workers[i] != NULL) {
            if (_workers[i]->isRunning()) {
              workerStillActive = true;
            } else {
              delete _workers[i];
              _workers[i] = NULL;
            }
          }
        }
        if (workerStillActive) vTaskDelay(1);
      } while (workerStillActive);
      delete _workers;
      _workers = NULL;
    } // if (_workers)

  }
}

/**
 * Adds a default header that is included in every response.
 *
 * This could be used for example to add a Server: header or for CORS options
 */
void HTTPServer::setDefaultHeader(std::string name, std::string value) {
  _defaultHeaders.set(new HTTPHeader(name, value));
}

/**
 * Manages server connections 
 *  - Cleans up closed connections
 *  - Queues work if there is new data on existing connection
 *  - Checks for new connections
 *  - Accepts pending connections when there is space in connection pool
 *  - Closes idle connections when there are pending ones
 *  
 *  Returns after all needed work is done or maxTimeoutMs expires
 */
void HTTPServer::manageConnections(int maxTimeoutMs) {
  fd_set readFDs, exceptFDs, timeoutFDs;
  FD_ZERO(&readFDs);
  FD_ZERO(&exceptFDs);
  FD_ZERO(&timeoutFDs);
  int maxSocket = -1;

  // The idea here is to block on something (up to maxTimeoutMs) until 
  // there is new data or new connection, so work can be queue up

  // Add only the server socket or the pending connection socket
  // as trying to select on the server socket while we know that
  // there is pending connection will return imediatelly
  if (_pendingConnection) {
    // If there is pending connection and we have not yet received data
    if (!_lookForIdleConnection) {
      int pendingSocket = _pendingConnection->getSocket();
      FD_SET(pendingSocket, &readFDs);
      FD_SET(pendingSocket, &exceptFDs);
      maxSocket = pendingSocket;
    }
  } else {
    // No pending connections (that we know of), monitor server socket
    FD_SET(_socket, &readFDs);
    FD_SET(_socket, &exceptFDs);
    maxSocket = _socket;
  }

  // Cleanup closed connections and find minimal select timeout
  // Add active connections to select sets
  int minRemain = maxTimeoutMs;
  for (int i = 0; i < _maxConnections; i++) {
    if (_connections[i] != NULL) {
      // Try to lock connection.
      if (xSemaphoreTake(_connectionMutex[i], 0)) {
        // If we suceeded, connection is currently not beening worked by other task
        int fd = _connections[i]->getSocket();
        if (_connections[i]->isClosed()) {
          // if it's closed clean up:
          HTTPS_LOGV("Deleted connection[%d], FID=%d", i, fd);
          delete _connections[i];
          _connections[i] = NULL;
           // We released one connection slot, don't look for idle connection
          _lookForIdleConnection = false;
          fd = -1;
        }
        if (fd > 0) {
          int remain = _connections[i]->remainingMsUntilTimeout();
          if (_lookForIdleConnection) {
            // There is partially accepted pending connection, check for idle connections
            if ((remain < 1) || _connections[i]->isIdle()) {
              HTTPS_LOGI("Closing IDLE connection[%d] FID=%d to accept FID=%d", i, fd, _pendingConnection->getSocket());
              _connections[i]->closeConnection();
              // We closed one connection, don't look for more idle connections
              _lookForIdleConnection = false;              
              fd = _connections[i]->getSocket();
            } else { 
              remain = min(remain, (int)HTTPS_CONNECTION_IDLE_TIMEOUT);
            }
          }
          if (fd > 0) {
            // Add the connection to select sets
            if (remain < 1) FD_SET(fd, &timeoutFDs);
            FD_SET(fd, &readFDs);
            FD_SET(fd, &exceptFDs);
            if (fd > maxSocket) maxSocket = fd;      
          } else {
            remain = 0; // Force imediate rescan
          }
          if (remain < minRemain) minRemain = remain;
        }
        xSemaphoreGive(_connectionMutex[i]);        
      }
    }
  }

  // Select on socket sets with minRemain (ms) timeout
  if (minRemain < 0) minRemain = 0;
  timeval _timeout;
  _timeout.tv_sec  = minRemain / 1000;
  _timeout.tv_usec = (minRemain - _timeout.tv_sec * 1000) * 1000;
  select(maxSocket + 1, &readFDs, NULL, &exceptFDs, &_timeout);

  // if FD_ISSET(serverSocket, &except_fds) {} // server is stopping ?

  // Assign work for connections that have data, error or timeout
  // and find empty connection slot 
  int8_t freeIndex = -1;
  for (int8_t i = 0; i < _maxConnections; i++) {
    if (_connections[i] != NULL) {
      int fd = _connections[i]->getSocket();
      if ((fd < 1) || (FD_ISSET(fd, &readFDs)) || (FD_ISSET(fd, &exceptFDs)) || (FD_ISSET(fd, &timeoutFDs))) {
        xQueueSend(_workQueue, &i, 0);
        HTTPS_LOGV("Queued work for connection[%d], FID=%d", i, fd);
      }
    } else {
      freeIndex = i;
    }
  }

  // If we have known pending connection ...
  if (_pendingConnection) {
    int pendingSocket = _pendingConnection->getSocket();
    // ... and if it is talking to us (client speaks first for both HTTP and TLS) ...
    if (_lookForIdleConnection || (FD_ISSET(pendingSocket, &readFDs))) {
      // ... and if we have space to fully accept ...
      if (freeIndex >= 0) {
        // ... try to fully accept the connection.
        if (_pendingConnection->fullyAccept() > 0) {
          // Fully accepted, add to active connections
          HTTPS_LOGV("Accepted connection[%d], FID=%d", freeIndex, pendingSocket);
          _connections[freeIndex] = _pendingConnection;
          xQueueSend(_workQueue, &freeIndex, 0);
        } else {
          HTTPS_LOGD("Discarded connection FID=%d", pendingSocket);
          delete _pendingConnection;
        }
        _pendingConnection = NULL;
        _lookForIdleConnection = false;
      } else {
        // Pending connection has data to read but we currently 
        // have no space in connection pool... set flag to try 
        // to close one of the idle connections...
        _lookForIdleConnection = true;
      }    
    }
  } else {
    // No pending connection, see if we have new one on the server socket
    if (FD_ISSET(_socket, &readFDs)) {
      // Try to initially accept the new connection
      HTTPConnection * connection = createConnection();
      int newSocket = (connection) ? connection->initialAccept() : -1;
      if (newSocket > 0) {
        // Initial accept succeded, do we have space in the pool
        if (freeIndex >= 0) {
          if (connection->fullyAccept() > 0) {
            _connections[freeIndex] = connection;
            xQueueSend(_workQueue, &freeIndex, 0);
            HTTPS_LOGV("Accepted pending connection[%d], FID=%d", freeIndex, newSocket);
          } else {
            HTTPS_LOGD("Discarded pending connection, FID=%d", newSocket);
            delete connection;
          }
        } else {
          // No space in the connection pool, keep it as pending connection
          // until it actually sends data (HTTP request or TLS 'hello')
          HTTPS_LOGV("Connection is pending, FID=%d", newSocket);
          _pendingConnection = connection;
        }
      } else {
        // Discard new connection imediatelly
        delete connection;
      }
    }
  } // if/else (_pendingConnection)

} // manageConnections()

/**
 * Pick up item (connection index) from work queue and 
 * call connection's loop method to consume the data on the socket
 * 
 * Returns false if there was no work in the queue and timeout expired
 */
bool HTTPServer::doQueuedWork(TickType_t waitDelay) {
  int8_t connIndex = -1;
  if (xQueueReceive(_workQueue, &connIndex, waitDelay)) {
    if ((connIndex >= 0) && xSemaphoreTake(_connectionMutex[connIndex], portMAX_DELAY)) {
      HTTPConnection * connection = _connections[connIndex];
      // Work the connection until it runs out of data
      while (connection && connection->loop()); 
      xSemaphoreGive(_connectionMutex[connIndex]);
    } 
    return true;
  } 
  return false;
}

/**
 * The loop method handles processing of data and should be called by periodicaly 
 * from the main loop when there are no workers enabled
 * 
 * If timeout (in millisecods) is supplied, it will wait for event 
 * on the 'server soceket' for new connection and/or on established 
 * connection sockets for closing/error.
 * 
 * Return value is remaining milliseconds if funtion returned early
 * 
 * NOTE: When workers are enabled, calling this method periodically
 *       is not needed and has no effect.
 */
int HTTPServer::loop(int timeoutMs) {

  // Only handle requests if the server is still running
  // and we are handling connections in async mode
  if (!_running || (_numWorkers > 0)) {
    delay(timeoutMs);
    return 0;
  }
  uint32_t startMs = millis();

  // Step 1: Process existing connections
  manageConnections(timeoutMs);

  // Step 2: Complete any remaining work (without waiting)
  while (doQueuedWork(0));

  // Return the remaining time from the timeoutMs requested
  uint32_t deltaMs = (startMs + timeoutMs - millis());
  if (deltaMs > 0x7FFFFFFF) deltaMs = 0;
  return deltaMs;
}

/**
 * Create new connection, initialize headers and return the pointer
 */
HTTPConnection * HTTPServer::createConnection() {
  HTTPConnection * newConnection = new HTTPConnection(this);
  newConnection->initialize(_socket, &_defaultHeaders);  
  return newConnection;  
}

/**
 * This method prepares the tcp server socket
 */
uint8_t HTTPServer::setupSocket() {
  // (AF_INET = IPv4, SOCK_STREAM = TCP)
  _socket = socket(AF_INET, SOCK_STREAM, 0);

  if (_socket>=0) {
    _sock_addr.sin_family = AF_INET;
    // Listen on all interfaces
    _sock_addr.sin_addr.s_addr = _bindAddress;
    // Set the server port
    _sock_addr.sin_port = htons(_port);

    // Now bind the TCP socket we did create above to the socket address we specified
    // (The TCP-socket now listens on 0.0.0.0:port)
    int err = bind(_socket, (struct sockaddr* )&_sock_addr, sizeof(_sock_addr));
    if(!err) {
      err = listen(_socket, _maxConnections);
      if (!err) {
        return 1;
      } else {
        close(_socket);
        _socket = -1;
        return 0;
      }
    } else {
      close(_socket);
      _socket = -1;
      return 0;
    }
  } else {
    _socket = -1;
    return 0;
  }
 
}

void HTTPServer::teardownSocket() {
  // Close the actual server sockets
  close(_socket);
  _socket = -1;
}

int HTTPServer::serverSocket() {
  return _socket;
}

} /* namespace httpsserver */
