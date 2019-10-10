#include "HTTPSConnection.hpp"

namespace httpsserver {


HTTPSConnection::HTTPSConnection(ResourceResolver * resResolver):
  HTTPConnection(resResolver) {
  _sslCtx = NULL;
  _ssl = NULL;
  _TLSTickets = NULL;
}

HTTPSConnection::~HTTPSConnection() {
  // Close the socket
  closeConnection();
}

bool HTTPSConnection::isSecure() {
  return true;
}

/**
 * Initializes the connection with SSL context
 */
void HTTPSConnection::initialize(int serverSocketID, HTTPHeaders *defaultHeaders, SSL_CTX * sslCtx, TLSTickets * tickets) {
  HTTPConnection::initialize(serverSocketID, defaultHeaders);
  _sslCtx = sslCtx;
  _TLSTickets = tickets;
}

/**
 * Accepts the connection from a server socket.
 *
 * The call WILL BLOCK if accept(serverSocketID) blocks. So use select() to check for that in advance.
 */
int HTTPSConnection::fullyAccept() {

  if (_connectionState == STATE_UNDEFINED) {
    initialAccept();
  }
  
  if (_connectionState == STATE_ACCEPTED) {
    int resSocket = _socket;

    // Build up SSL Connection context if the socket has been created successfully
    if (resSocket >= 0) {
      HTTPS_LOGV("Before SSL accept free:%u, lfb:%u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
      _ssl = SSL_new(_sslCtx);

      if (_TLSTickets != NULL) _TLSTickets->enable(_ssl);

      if (_ssl) {
        // Bind SSL to the socket
        int success = SSL_set_fd(_ssl, resSocket);
        if (success) {

          // Perform the handshake
          success = SSL_accept(_ssl);
          if (success) {
            HTTPS_LOGD("SSL accepted (FID=%d)", resSocket);
            HTTPS_LOGV("After SSL accept free:%u, lfb:%u", heap_caps_get_free_size(MALLOC_CAP_8BIT), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
            _connectionState = STATE_INITIAL;
            return resSocket;
          } else {
            HTTPS_LOGE("SSL_accept failed. Aborting handshake. FID=%d", resSocket);
            HTTPS_LOGV("After fail free:%u, lfb:%u", heap_caps_get_free_size(MALLOC_CAP_8BIT), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
          }
        } else {
          HTTPS_LOGE("SSL_set_fd failed. Aborting handshake. FID=%d", resSocket);
        }
      } else {
        HTTPS_LOGE("SSL_new failed. Aborting handshake. FID=%d", resSocket);
      }

    } else {
      HTTPS_LOGE("Could not accept() new connection. FID=%d", resSocket);
    }

    _connectionState = STATE_ERROR;
    _clientState = CSTATE_ACTIVE;

    // This will only be called if the connection could not be established and cleanup
    // variables like _ssl etc.
    closeConnection();
  }
  // Error: The connection has already been established or could not be established
  return -1;
}


void HTTPSConnection::closeConnection() {

  // FIXME: Copy from HTTPConnection, could be done better probably
  if (_connectionState != STATE_ERROR && _connectionState != STATE_CLOSED) {

    // First call to closeConnection - set the timestamp to calculate the timeout later on
    if (_connectionState != STATE_CLOSING) {
      _shutdownTS = millis();
    }

    // Set the connection state to closing. We stay in closing as long as SSL has not been shutdown
    // correctly
    _connectionState = STATE_CLOSING;
  }

  // Try to tear down SSL while we are in the _shutdownTS timeout period or if an error occurred
  if (_ssl) {
    if(_connectionState == STATE_ERROR || SSL_shutdown(_ssl) == 0) {
      // SSL_shutdown will return 1 as soon as the client answered with close notify
      // This means we are safe to close the socket
      SSL_free(_ssl);
      _ssl = NULL;
    } else if (_shutdownTS + HTTPS_SHUTDOWN_TIMEOUT < millis()) {
      // The timeout has been hit, we force SSL shutdown now by freeing the context
      SSL_free(_ssl);
      _ssl = NULL;
      HTTPS_LOGW("SSL_shutdown did not receive close notification from the client");
      _connectionState = STATE_ERROR;
    }
  }

  // If SSL has been brought down, close the socket
  if (!_ssl) {
    HTTPConnection::closeConnection();
  }
}

size_t HTTPSConnection::writeBuffer(byte* buffer, size_t length) {
  if (_ssl != NULL) return SSL_write(_ssl, buffer, length);
  return 0;
}

size_t HTTPSConnection::readBytesToBuffer(byte* buffer, size_t length) {
  if (_ssl != NULL) return SSL_read(_ssl, buffer, length);
  return 0;
}

size_t HTTPSConnection::pendingByteCount() {
  return (_ssl != NULL) && SSL_pending(_ssl);
}

bool HTTPSConnection::canReadData() {
  return HTTPConnection::canReadData() || ((_ssl != NULL) && (SSL_pending(_ssl) > 0));
}

} /* namespace httpsserver */
