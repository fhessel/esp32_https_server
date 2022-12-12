#include "HTTPSServer.hpp"

namespace httpsserver {

constexpr const char * alpn_protos[] = { "http/1.1", NULL } ;

HTTPSServer::HTTPSServer(SSLCert * cert, const uint16_t port, const uint8_t maxConnections, const in_addr_t bindAddress):
  HTTPServer(port, maxConnections, bindAddress),
  _cert(cert) {
  // Configure runtime data
  _cfg = new esp_tls_cfg_server();
  _cfg->alpn_protos = (const char **)alpn_protos;
  _cfg->cacert_buf = NULL;
  _cfg->cacert_bytes = 0;
  _cfg->servercert_buf =cert->getCertData();
  _cfg->servercert_bytes = cert->getCertLength();
  _cfg->serverkey_buf= cert->getPKData();
  _cfg->serverkey_bytes= cert->getPKLength();
}

HTTPSServer::~HTTPSServer() {
  free(_cfg);
}

/**
 * This method starts the server and begins to listen on the port
 */
uint8_t HTTPSServer::setupSocket() {
  if (!isRunning()) {
    _cfg->servercert_buf= _cert->getCertData();
    _cfg->servercert_bytes = _cert->getCertLength();
    _cfg->serverkey_buf= _cert->getPKData();
    _cfg->serverkey_bytes= _cert->getPKLength();

    if (HTTPServer::setupSocket()) {
      return 1;
    } else {
      Serial.println("setupSockets failed");
      return 0;
    }
  } else {
    return 1;
  }
}

void HTTPSServer::teardownSocket() {

  HTTPServer::teardownSocket();

}

int HTTPSServer::createConnection(int idx) {
  HTTPSConnection * newConnection = new HTTPSConnection(this);
  _connections[idx] = newConnection;
  return newConnection->initialize(_socket, _cfg , &_defaultHeaders);
}

/**
 * This method configures the certificate and private key for the given
 * ssl context
 */
uint8_t HTTPSServer::setupCert() {
  // Configure the certificate first
  _cfg->servercert_buf= _cert->getCertData();
  _cfg->servercert_bytes = _cert->getCertLength();
  _cfg->serverkey_buf= _cert->getPKData();
  _cfg->serverkey_bytes= _cert->getPKLength();
  return 1;
}

} /* namespace httpsserver */
