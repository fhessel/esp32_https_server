#include "HTTPServer.hpp"

namespace httpsserver {


HTTPServer::HTTPServer(const uint16_t port, const uint8_t maxConnections, const in_addr_t bindAddress):
  _port(port),
  _maxConnections(maxConnections),
  _bindAddress(bindAddress) {

  // Create space for the connections
  _connections = new HTTPConnection*[maxConnections];
  for(uint8_t i = 0; i < maxConnections; i++) _connections[i] = NULL;

  // Configure runtime data
  _socket = -1;
  _running = false;
}

HTTPServer::~HTTPServer() {

  // Stop the server.
  // This will also remove all existing connections
  if(_running) {
    stop();
  }

  // Delete connection pointers
  delete[] _connections;
}

/**
 * This method starts the server and begins to listen on the port
 */
uint8_t HTTPServer::start() {
  if (!_running) {
    if (setupSocket()) {
      _running = true;
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

    // Clean up the connections
    bool hasOpenConnections = true;
    while(hasOpenConnections) {
      hasOpenConnections = false;
      for(int i = 0; i < _maxConnections; i++) {
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
      }
      delay(1);
    }

    teardownSocket();

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
 * The loop method can either be called by periodical interrupt or in the main loop and handles processing
 * of data
 */
void HTTPServer::loop() {

  // Only handle requests if the server is still running
  if(!_running) return;

  // Step 1: Process existing connections
  // Process open connections and store the index of a free connection
  // (we might use that later on)
  int freeConnectionIdx = -1;
  for (int i = 0; i < _maxConnections; i++) {
    // Fetch a free index in the pointer array
    if (_connections[i] == NULL) {
      freeConnectionIdx = i;

    } else {
      // if there is a connection (_connections[i]!=NULL), check if its open or closed:
      if (_connections[i]->isClosed()) {
        // if it's closed, clean up:
        delete _connections[i];
        _connections[i] = NULL;
        freeConnectionIdx = i;
      } else {
        // if not, process it:
        _connections[i]->loop();
      }
    }
  }
 
  // Step 2: Check for new connections
  // This makes only sense if there is space to store the connection
  if (freeConnectionIdx > -1) {

    // We create a file descriptor set to be able to use the select function
    fd_set sockfds;
    // Out socket is the only socket in this set
    FD_ZERO(&sockfds);
    FD_SET(_socket, &sockfds);

    // We define a "immediate" timeout
    timeval timeout;
    timeout.tv_sec  = 0;
    timeout.tv_usec = 0; // Return immediately, if possible

    // Wait for input
    // As by 2017-12-14, it seems that FD_SETSIZE is defined as 0x40, but socket IDs now
    // start at 0x1000, so we need to use _socket+1 here
    select(_socket + 1, &sockfds, NULL, NULL, &timeout);

    // There is input
    if (FD_ISSET(_socket, &sockfds)) {
      int socketIdentifier = createConnection(freeConnectionIdx);

      // If initializing did not work, discard the new socket immediately
      if (socketIdentifier < 0) {
        delete _connections[freeConnectionIdx];
        _connections[freeConnectionIdx] = NULL;
      }
    }

  }
}

int HTTPServer::createConnection(int idx) {
  HTTPConnection * newConnection = new HTTPConnection(this);
  _connections[idx] = newConnection;
  return newConnection->initialize(_socket, &_defaultHeaders);
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

} /* namespace httpsserver */
