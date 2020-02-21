#include "HTTPSServer.hpp"

namespace httpsserver {


HTTPSServer::HTTPSServer(SSLCert * cert, const uint16_t port, const uint8_t maxConnections, const in_addr_t bindAddress):
  HTTPServer(port, maxConnections, bindAddress),
  _cert(cert) {

  // Configure runtime data
  _sslctx = NULL;
}

HTTPSServer::~HTTPSServer() {

}

/**
 * This method starts the server and begins to listen on the port
 */
uint8_t HTTPSServer::setupSocket() {
  if (!isRunning()) {
    if (!setupSSLCTX()) {
      Serial.println("setupSSLCTX failed");
      return 0;
    }

    if (!setupCert()) {
      Serial.println("setupCert failed");
      SSL_CTX_free(_sslctx);
      _sslctx = NULL;
      return 0;
    }

    if (HTTPServer::setupSocket()) {
      return 1;
    } else {
      Serial.println("setupSockets failed");
      SSL_CTX_free(_sslctx);
      _sslctx = NULL;
      return 0;
    }
  } else {
    return 1;
  }
}

void HTTPSServer::teardownSocket() {

  HTTPServer::teardownSocket();

  // Tear down the SSL context
  SSL_CTX_free(_sslctx);
  _sslctx = NULL;
}

int HTTPSServer::createConnection(int idx) {
  HTTPSConnection * newConnection = new HTTPSConnection(this);
  _connections[idx] = newConnection;
  return newConnection->initialize(_socket, _sslctx, &_defaultHeaders);
}

/**
 * This method configures the ssl context that is used for the server
 */
uint8_t HTTPSServer::setupSSLCTX() {
  _sslctx = SSL_CTX_new(TLSv1_2_server_method());
  if (_sslctx) {
    // Set SSL Timeout to 5 minutes
    SSL_CTX_set_timeout(_sslctx, 300);
    return 1;
  } else {
    _sslctx = NULL;
    return 0;
  }
}

/**
 * This method configures the certificate and private key for the given
 * ssl context
 */
uint8_t HTTPSServer::setupCert() {
  // Configure the certificate first
  uint8_t ret = SSL_CTX_use_certificate_ASN1(
    _sslctx,
    _cert->getCertLength(),
    _cert->getCertData()
  );

  // Then set the private key accordingly
  if (ret) {
    ret = SSL_CTX_use_RSAPrivateKey_ASN1(
      _sslctx,
      _cert->getPKData(),
      _cert->getPKLength()
    );
  }

  return ret;
}

} /* namespace httpsserver */
