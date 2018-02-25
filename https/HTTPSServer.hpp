/*
 * HTTPSServer.hpp
 *
 *  Created on: Dec 6, 2017
 *      Author: frank
 */

#ifndef HTTPS_HTTPSSERVER_HPP_
#define HTTPS_HTTPSSERVER_HPP_

// Standard library
#include <string>

// Arduino stuff
#include <Arduino.h>

// Required for SSL
#include "openssl/ssl.h"
#undef read

// Required for sockets
#include "lwip/netdb.h"
#undef read
#include "lwip/sockets.h"
#include "lwip/inet.h"

// Internal includes
#include "HTTPSServerConstants.hpp"
#include "ResourceNode.hpp"
#include "ResourceResolver.hpp"
#include "ResolvedResource.hpp"
#include "HTTPSConnection.hpp"
#include "SSLCert.hpp"

namespace httpsserver {

class HTTPSServer : public ResourceResolver {
public:
	HTTPSServer(SSLCert * cert, const uint16_t port = 443, const uint8_t maxConnections = 4, const in_addr_t bindAddress = 0);
	virtual ~HTTPSServer();

	uint8_t start();
	void stop();
	bool isRunning();

	void loop();

private:
	// Static configuration. Port, keys, etc. ====================
	// Certificate that should be used (includes private key)
	SSLCert * _cert;
	// Port that the server will listen on
	const uint16_t _port;
	// Max parallel connections that the server will accept
	const uint8_t _maxConnections;
	// Address to bind to (0 = all interfaces)
	const in_addr_t _bindAddress;

	//// Runtime data ============================================
	// The array of connections that are currently active
	HTTPSConnection ** _connections;
	// The SSL context based on the cert and private key
	SSL_CTX * _sslctx;
	// Status of the server: Are we running, or not?
	boolean _running;
	// The server socket
	int _socket;
	// The server socket address, that our service is bound to
	sockaddr_in _sock_addr;

	// Setup functions
	uint8_t setupSSLCTX();
	uint8_t setupCert();
	uint8_t setupSocket();
};

} /* namespace httpsserver */

#endif /* HTTPS_HTTPSSERVER_HPP_ */
