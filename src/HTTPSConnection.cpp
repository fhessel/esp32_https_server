#include "HTTPSConnection.hpp"

namespace httpsserver {


HTTPSConnection::HTTPSConnection(ResourceResolver * resResolver):
  HTTPConnection(resResolver) {
  _ssl = esp_tls_init();
}

HTTPSConnection::~HTTPSConnection() {
  // Close the socket
  closeConnection();
}

bool HTTPSConnection::isSecure() {
  return true;
}

/**
 * Initializes the connection from a server socket.
 *
 * The call WILL BLOCK if accept(serverSocketID) blocks. So use select() to check for that in advance.
 */
int HTTPSConnection::initialize(int serverSocketID, esp_tls_cfg_server_t * cfgSrv, HTTPHeaders *defaultHeaders) {
  if (_connectionState == STATE_UNDEFINED) {
    // Let the base class connect the plain tcp socket
    int resSocket = HTTPConnection::initialize(serverSocketID, defaultHeaders);
    HTTPS_LOGI("Cert len:%d, apn:%s\n",cfgSrv->servercert_bytes,cfgSrv->alpn_protos[0]);
    // Build up SSL Connection context if the socket has been created successfully
    if (resSocket >= 0) {
      int res=esp_tls_server_session_create(cfgSrv,resSocket,_ssl);
      if (0==res) {
        esp_tls_cfg_server_session_tickets_init(cfgSrv);
        _cfg = cfgSrv;
        // Bind SSL to the socket
        if (ESP_OK == esp_tls_get_conn_sockfd(_ssl,&resSocket)) {
            return resSocket;
        } else {
             HTTPS_LOGE("SSL_accept failed. Aborting handshake. FID=%d", resSocket);
        }
      } else {
        HTTPS_LOGE("SSL_new failed. Aborting handshake. Error=%d", res);
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
    esp_tls_cfg_server_session_tickets_free(_cfg);
    esp_tls_server_session_delete(_ssl);
    _ssl = NULL;
    _connectionState = STATE_ERROR;
  }

  // If SSL has been brought down, close the socket
  if (!_ssl) {
    HTTPConnection::closeConnection();
  }
}

size_t HTTPSConnection::writeBuffer(byte* buffer, size_t length) {
  return esp_tls_conn_write(_ssl,buffer,length);// SSL_write(_ssl, buffer, length);
}

size_t HTTPSConnection::readBytesToBuffer(byte* buffer, size_t length) {
  return esp_tls_conn_read(_ssl, buffer, length);
}

size_t HTTPSConnection::pendingByteCount() {
  return esp_tls_get_bytes_avail(_ssl);
}

bool HTTPSConnection::canReadData() {
  return HTTPConnection::canReadData() || (esp_tls_get_bytes_avail(_ssl) > 0);
}

} /* namespace httpsserver */
