/*
 * HTTPSConnection.cpp
 *
 *  Created on: Dec 6, 2017
 *      Author: frank
 */

#include "HTTPSConnection.hpp"

namespace httpsserver {


HTTPSConnection::HTTPSConnection(ResourceResolver * resResolver):
	_resResolver(resResolver) {
	_socket = -1;
	_addrLen = 0;
	_ssl = NULL;

	_bufferProcessed = 0;
	_bufferUnusedIdx = 0;

	_connectionState = STATE_UNDEFINED;
	_clientState = CSTATE_UNDEFINED;
	_httpHeaders = NULL;
	_isKeepAlive = false;
	_lastTransmissionTS = millis();
}

HTTPSConnection::~HTTPSConnection() {
	// Close the socket
	closeConnection();
}

/**
 * Initializes the connection from a server socket.
 *
 * The call WILL BLOCK if accept(serverSocketID) blocks. So use select() to check for that in advance.
 */
int HTTPSConnection::initialize(int serverSocketID, SSL_CTX * sslCtx) {
	if (_connectionState == STATE_UNDEFINED) {
		_socket = accept(serverSocketID, (struct sockaddr * )&_sockAddr, &_addrLen);

		// Build up SSL Connection context if the socket has been created successfully
		if (_socket >= 0) {
			HTTPS_DLOGHEX("[-->] New connection. Socket fid is: ", _socket);

			_ssl = SSL_new(sslCtx);

			// Bind SSL to the socket
			int success = SSL_set_fd(_ssl, _socket);
			if (success) {

				// Perform the handshake
				success = SSL_accept(_ssl);
				if (success) {
					_connectionState = STATE_INITIAL;
					_httpHeaders = new HTTPHeaders();
					refreshTimeout();
					return _socket;
				}
			}
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


/**
 * True if the connection is timed out.
 *
 * (Should be checkd in the loop and transition should go to CONNECTION_CLOSE if exceeded)
 */
bool HTTPSConnection::isTimeoutExceeded() {
	return _lastTransmissionTS + HTTPS_CONNECTION_TIMEOUT < millis();
}

/**
 * Resets the timeout to allow again the full HTTPS_CONNECTION_TIMEOUT milliseconds
 */
void HTTPSConnection::refreshTimeout() {
	_lastTransmissionTS = millis();
}

/**
 * Returns true, if the connection has been closed.
 */
bool HTTPSConnection::isClosed() {
	return (_connectionState == STATE_ERROR || _connectionState == STATE_CLOSED);
}

/**
 * Returns true, if the connection has been closed due to error
 */
bool HTTPSConnection::isError() {
	return (_connectionState == STATE_ERROR);
}

void HTTPSConnection::closeConnection() {
	// TODO: Call an event handler here, maybe?

	// Tear down SSL
	if (_ssl) {
		// wait here so that SSL_shutdown returns 1
		while(SSL_shutdown(_ssl) == 0) delay(1);
		SSL_free(_ssl);
		_ssl = NULL;
	}

	// Tear down the socket
	if (_socket >= 0) {
		HTTPS_DLOGHEX("[<--] Connection has been closed. fid = ", _socket);
		close(_socket);
		_socket = -1;
		_addrLen = 0;
	}

	if (_connectionState != STATE_ERROR) {
		_connectionState = STATE_CLOSED;
	}

	if (_httpHeaders != NULL) {
		delete _httpHeaders;
		_httpHeaders = NULL;
	}
}

/**
 * This method will try to fill up the buffer with data from
 */
int HTTPSConnection::updateBuffer() {
	if (!isClosed()) {

		// If there is buffer data that has been marked as processed.
		// Some example is shown here:
		//
		// Previous configuration:
		// GET / HTTP/1.1\\Host: test\\Foo: bar\\\\[some uninitialized memory]
		//                 ^ processed             ^ unusedIdx
		//
		// New configuration after shifting:
		// Host: test\\Foo: bar\\\\[some uninitialized memory]
		// ^ processed             ^ unusedIdx
		if (_bufferProcessed > 0) {
			for(int i = 0; i < HTTPS_CONNECTION_DATA_CHUNK_SIZE; i++) {
				int copyFrom = i + _bufferProcessed;
				if (copyFrom < _bufferUnusedIdx) {
					_receiveBuffer[i] = _receiveBuffer[copyFrom];
				} else {
					break;
				}
			}
			_bufferUnusedIdx -= _bufferProcessed;
			_bufferProcessed = 0;

		}

		if (_bufferUnusedIdx < HTTPS_CONNECTION_DATA_CHUNK_SIZE) {
			// Check only this socket for data
			fd_set sockfds;
			FD_ZERO( &sockfds );
			FD_SET(_socket, &sockfds);

			// We define an immediate timeout (return immediately, if there's no data)
			timeval timeout;
			timeout.tv_sec  = 0;
			timeout.tv_usec = 0;

			// Check for input
			// As by 2017-12-14, it seems that FD_SETSIZE is defined as 0x40, but socket IDs now
			// start at 0x1000, so we need to use _socket+1 here
			select(_socket + 1, &sockfds, NULL, NULL, &timeout);

			// TODO: Check which one is relevant here (maybe both)
			if (FD_ISSET(_socket, &sockfds) || SSL_pending(_ssl) > 0) {
				HTTPS_DLOGHEX("[   ] There is data on the connection socket. fid=", _socket)

				// The return code of SSL_read means:
				// > 0 : Length of the data that has been read
				// < 0 : Error
				// = 0 : Connection closed
				int readReturnCode = SSL_read(
						_ssl,
						// Only after the part of the buffer that has not been processed yet
						_receiveBuffer + sizeof(char) * _bufferUnusedIdx,
						// Only append up to the end of the buffer
						HTTPS_CONNECTION_DATA_CHUNK_SIZE - _bufferUnusedIdx
				);

				if (readReturnCode > 0) {
					_bufferUnusedIdx += readReturnCode;
					refreshTimeout();
					return readReturnCode;

				} else if (readReturnCode == 0) {
					// The connection has been closed by the client
					_clientState = CSTATE_CLOSED;
					HTTPS_DLOGHEX("[ x ] Client closed connection, fid=", _socket);
					// TODO: If we are in state websocket, we might need to do something here
					return 0;
				} else {
					// An error occured
					_connectionState = STATE_ERROR;
					HTTPS_DLOGHEX("[ERR] An SSL error occured, fid=", _socket);
					closeConnection();
					return -1;
				}
			}
		}
	}
	return 0;
}

void HTTPSConnection::serverError() {
	_connectionState = STATE_ERROR;

	// TODO: Write 500
	Serial.println("Server error");
	char staticResponse[] = "HTTP/1.1 500 Internal Server Error\r\nServer: esp32https\r\nConnection:close\r\nContent-Type: text/html\r\nContent-Length:34\r\n\r\n<h1>500 Internal Server Error</h1>";
	SSL_write(_ssl, staticResponse, strlen(staticResponse));

	closeConnection();
}


void HTTPSConnection::clientError() {
	_connectionState = STATE_ERROR;

	// TODO: Write 400
	Serial.println("Client error");
	char staticResponse[] = "HTTP/1.1 400 Bad Request\r\nServer: esp32https\r\nConnection:close\r\nContent-Type: text/html\r\nContent-Length:26\r\n\r\n<h1>400 Bad Request</h1>";
	SSL_write(_ssl, staticResponse, strlen(staticResponse));

	closeConnection();
}

void HTTPSConnection::readLine(int lengthLimit) {
	while(_bufferProcessed < _bufferUnusedIdx) {
		char newChar = _receiveBuffer[_bufferProcessed];

		if ( newChar == '\r') {
			// Look ahead for \n (if not possible, wait for next round
			if (_bufferProcessed+1 < _bufferUnusedIdx) {
				if (_receiveBuffer[_bufferProcessed+1] == '\n') {
					_bufferProcessed += 2;
					_parserLine.parsingFinished = true;
					return;
				} else {
					// Line has not been terminated by \r\n
					HTTPS_DLOG("[ERR] Line that has not been terminated by \\r\\n (got only \\r). Client error.");
					clientError();
					return;
				}
			}
		} else {
			_parserLine.text += newChar;
			_bufferProcessed += 1;
		}

		// Check that the max request string size is not exceeded
		if (_parserLine.text.length() > lengthLimit) {
			HTTPS_DLOG("[ERR] Line length exceeded. Server error.");
			serverError();
			return;
		}
	}
}

/**
 * Called by the request to signal that the client has closed the connection
 */
void HTTPSConnection::signalClientClose() {
	_clientState = CSTATE_CLOSED;
}

/**
 * Called by the request to signal that an error has occured
 */
void HTTPSConnection::signalRequestError() {
	// TODO: Check that no response has been transmitted yet
	serverError();
}

/**
 * Returns the cache size that should be cached (in the response) to enable keep-alive requests.
 *
 * 0 = no keep alive.
 */
size_t HTTPSConnection::getCacheSize() {
	return (_isKeepAlive ? HTTPS_KEEPALIVE_CACHESIZE : 0);
}

void HTTPSConnection::loop() {
	// First, update the buffer
	// newByteCount will contain the number of new bytes that have to be processed
	updateBuffer();

	if (_clientState == CSTATE_CLOSED) {
		HTTPS_DLOGHEX("[   ] Client closed in state", _clientState)
	}

	if (_clientState == CSTATE_CLOSED && _bufferProcessed == _bufferUnusedIdx && _connectionState < STATE_HEADERS_FINISHED) {
		closeConnection();
	}

	if (!isClosed() && isTimeoutExceeded()) {
		HTTPS_DLOGHEX("[zZz] Connection timeout exceeded, closing connection. fid=", _socket)
		closeConnection();
	}

	if (!isError()) {
		// State machine (Reading request, reading headers, ...)
		switch(_connectionState) {
		case STATE_INITIAL: // Read request line
			readLine(HTTPS_REQUEST_MAX_REQUEST_LENGTH);
			if (_parserLine.parsingFinished && !isClosed()) {
				// Find the method
				size_t spaceAfterMethodIdx = _parserLine.text.find(' ');
				if (spaceAfterMethodIdx == std::string::npos) {
					HTTPS_DLOG("[ERR] Missing space after HTTP method. Client Error.")
					clientError();
					break;
				}
				_httpMethod = _parserLine.text.substr(0, spaceAfterMethodIdx);

				// Find the resource string:
				size_t spaceAfterResourceIdx = _parserLine.text.find(' ', spaceAfterMethodIdx + 1);
				if (spaceAfterResourceIdx == std::string::npos) {
					HTTPS_DLOG("[ERR] Missing space after HTTP resource. Client Error.")
					clientError();
					break;
				}
				_httpResource = _parserLine.text.substr(spaceAfterMethodIdx + 1, spaceAfterResourceIdx - _httpMethod.length() - 1);

				_parserLine.parsingFinished = false;
				_parserLine.text = "";
				HTTPS_DLOG(("[   ] Request line finished: method="+_httpMethod+", resource="+_httpResource).c_str());
				_connectionState = STATE_REQUEST_FINISHED;
			}

			break;
		case STATE_REQUEST_FINISHED: // Read headers

			while (_bufferProcessed < _bufferUnusedIdx && !isClosed()) {
				readLine(HTTPS_REQUEST_MAX_HEADER_LENGTH);
				if (_parserLine.parsingFinished && _connectionState != STATE_ERROR) {

					if (_parserLine.text.empty()) {
						HTTPS_DLOG("[   ] Headers finished");
						_connectionState = STATE_HEADERS_FINISHED;
					} else {
						int idxColon = _parserLine.text.find(':');
						if ( (idxColon != std::string::npos) && (_parserLine.text[idxColon+1]==' ') ) {
							_httpHeaders->set(new HTTPHeader(
									_parserLine.text.substr(0, idxColon),
									_parserLine.text.substr(idxColon+2)
							));
						} else {
							HTTPS_DLOG("Malformed header line detected. Client error.");
							HTTPS_DLOG(_parserLine.text.c_str());
							clientError();
							break;
						}
					}

					_parserLine.parsingFinished = false;
					_parserLine.text = "";
				}
			}

			break;
		case STATE_HEADERS_FINISHED: // Handle body
			{
				HTTPS_DLOG("[   ] Resolving resource...");
				ResolvedResource resolvedResource;
				_resResolver->resolveNode(_httpMethod, _httpResource, resolvedResource);
				if (resolvedResource.didMatch()) {
					// Did the client request connection:keep-alive?
					HTTPHeader * connectionHeader = _httpHeaders->get("Connection");
					if (connectionHeader != NULL && std::string("keep-alive").compare(connectionHeader->_value)==0) {
						HTTPS_DLOGHEX("[   ] Keep-Alive activated. fid=", _socket);
						_isKeepAlive = true;
					} else {
						HTTPS_DLOGHEX("[   ] Keep-Alive disabled. fid=", _socket);
						_isKeepAlive = false;
					}

					// Create request context
					HTTPRequest req  = HTTPRequest(this, _httpHeaders, resolvedResource.getParams());
					HTTPResponse res = HTTPResponse(this);

					// Call the callback
					HTTPS_DLOG("[   ] Calling handler function");
					resolvedResource.getMatchingNode()->_callback(&req, &res);
					HTTPS_DLOG("[   ] Handler function done, requeste complete");

					if (!_isKeepAlive) {
						// No KeepAlive -> We are done. Transition to next state.
						if (!isClosed()) {
							_connectionState = STATE_BODY_FINISHED;
						}
					} else {
						if (res.isResponseBuffered()) {
							// If the response could be buffered:
							res.setHeader("Connection", "keep-alive");
							res.finalize();
							if (_clientState != CSTATE_CLOSED) {
								refreshTimeout();
								_connectionState = STATE_INITIAL;
							}
						}
						// The response could not be buffered or the client has closed:
						if (!isClosed() && _connectionState!=STATE_INITIAL) {
							_connectionState = STATE_BODY_FINISHED;
						}
					}
				} else {
					HTTPS_DLOG("[ERR] Could not find a matching resource. Server error.");
					serverError();
				}

			}
			break;
		case STATE_BODY_FINISHED: // Request is complete
			closeConnection();
			break;
		case STATE_WEBSOCKET: // Do handling of the websocket

			break;
		default:;
		}
	}

}

} /* namespace httpsserver */
