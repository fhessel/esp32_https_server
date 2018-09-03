/*
 * HTTPSServer.hpp
 *
 *  Created on: Dec 6, 2017
 *      Author: frank
 */

#ifndef SRC_HTTPSSERVER_HPP_
#define SRC_HTTPSSERVER_HPP_

// Standard library
#include <string>

// Arduino stuff
#include <Arduino.h>

// Required for SSL
#include "openssl/ssl.h"
#undef read

// Internal includes
#include "../src/HTTPServer.hpp"
#include "../src/HTTPSServerConstants.hpp"
#include "../src/HTTPHeaders.hpp"
#include "../src/HTTPHeader.hpp"
#include "../src/ResourceNode.hpp"
#include "../src/ResourceResolver.hpp"
#include "../src/ResolvedResource.hpp"
#include "../src/HTTPSConnection.hpp"
#include "../src/SSLCert.hpp"

namespace httpsserver {

class HTTPSServer : public HTTPServer {
public:
	HTTPSServer(SSLCert * cert, const uint16_t portHTTPS = 443, const uint8_t maxConnections = 4, const in_addr_t bindAddress = 0);
	virtual ~HTTPSServer();

private:
	// Static configuration. Port, keys, etc. ====================
	// Certificate that should be used (includes private key)
	SSLCert * _cert;
 
	//// Runtime data ============================================
	SSL_CTX * _sslctx;
	// Status of the server: Are we running, or not?

	// Setup functions
	virtual uint8_t setupSocket();
	virtual void teardownSocket();
	uint8_t setupSSLCTX();
	uint8_t setupCert();

	// Helper functions
	virtual int createConnection(int idx);
};

} /* namespace httpsserver */

#endif /* SRC_HTTPSSERVER_HPP_ */
