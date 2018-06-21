/*
 * HTTPSServer.cpp
 *
 *  Created on: Dec 6, 2017
 *      Author: frank
 */

#include "HTTPSServer.hpp"

namespace httpsserver {


HTTPSServer::HTTPSServer(SSLCert * cert, const uint16_t port, const uint8_t maxConnections, const in_addr_t bindAddress):
	_cert(cert),
	_port(port),
	_maxConnections(maxConnections),
	_bindAddress(bindAddress) {

	// Create space for the connections
	_connections = new HTTPSConnection*[maxConnections];
	for(uint8_t i = 0; i < maxConnections; i++) _connections[i] = NULL;

	// Configure runtime data
	_sslctx = NULL;
	_socket = -1;
	_running = false;
}

HTTPSServer::~HTTPSServer() {

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
uint8_t HTTPSServer::start() {
	if (!_running) {
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

		if (setupSocket()) {
			_running = true;
			return 1;
		} else {
			Serial.println("setupSocket failed");
			SSL_CTX_free(_sslctx);
			_sslctx = NULL;
			return 0;
		}
	} else {
		return 1;
	}
}

bool HTTPSServer::isRunning() {
	return _running;
}

/**
 * This method stops the server
 */
void HTTPSServer::stop() {

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
					} else {
						hasOpenConnections = true;
					}
				}
			}
			delay(1);
		}

		// Close the actual server socket
		close(_socket);
		_socket = -1;

		// Tear down the SSL context
		SSL_CTX_free(_sslctx);
		_sslctx = NULL;
	}
}

/**
 * Adds a default header that is included in every response.
 *
 * This could be used for example to add a Server: header or for CORS options
 */
void HTTPSServer::setDefaultHeader(std::string name, std::string value) {
	_defaultHeaders.set(new HTTPHeader(name, value));
}

/**
 * The loop method can either be called by periodical interrupt or in the main loop and handles processing
 * of data
 */
void HTTPSServer::loop() {

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

			_connections[freeConnectionIdx] = new HTTPSConnection(this);

			// Start to accept data on the socket
			int socketIdentifier = _connections[freeConnectionIdx]->initialize(_socket, _sslctx, &_defaultHeaders);

			// If initializing did not work, discard the new socket immediately
			if (socketIdentifier < 0) {
				delete _connections[freeConnectionIdx];
				_connections[freeConnectionIdx] = NULL;
			}
		}
	}


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

/**
 * This method prepares the tcp server socket
 */
uint8_t HTTPSServer::setupSocket() {
	// (AF_INET = IPv4, SOCK_STREAM = TCP)
	_socket = socket(AF_INET, SOCK_STREAM, 0);

	Serial.print("Server Socket fid=0x");
	Serial.println(_socket, HEX);

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

} /* namespace httpsserver */
