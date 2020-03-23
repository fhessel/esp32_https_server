#include "HTTPConnection.hpp"

namespace httpsserver {

HTTPConnection::HTTPConnection(ResourceResolver * resResolver):
  _resResolver(resResolver) {
  _socket = -1;
  _addrLen = 0;

  _bufferProcessed = 0;
  _bufferUnusedIdx = 0;

  _connectionState = STATE_UNDEFINED;
  _clientState = CSTATE_UNDEFINED;
  _httpHeaders = NULL;
  _defaultHeaders = NULL;
  _isKeepAlive = false;
  _lastTransmissionTS = millis();
  _shutdownTS = 0;
  _wsHandler = nullptr;
}

HTTPConnection::~HTTPConnection() {
  // Close the socket
  closeConnection();
}

/**
 * Initializes the connection from a server socket.
 *
 * The call WILL BLOCK if accept(serverSocketID) blocks. So use select() to check for that in advance.
 */
int HTTPConnection::initialize(int serverSocketID, HTTPHeaders *defaultHeaders) {
  if (_connectionState == STATE_UNDEFINED) {
    _defaultHeaders = defaultHeaders;
    _addrLen = sizeof(_sockAddr);
    _socket = accept(serverSocketID, (struct sockaddr * )&_sockAddr, &_addrLen);

    // Build up SSL Connection context if the socket has been created successfully
    if (_socket >= 0) {
      HTTPS_LOGI("New connection. Socket FID=%d", _socket);
      _connectionState = STATE_INITIAL;
      _httpHeaders = new HTTPHeaders();
      refreshTimeout();
      return _socket;
    }
     
    HTTPS_LOGE("Could not accept() new connection");
   
    _addrLen = 0;
    _connectionState = STATE_ERROR;
    _clientState = CSTATE_ACTIVE;

    // This will only be called if the connection could not be established and cleanup
    // variables  etc.
    closeConnection();
  }
  // Error: The connection has already been established or could not be established
  return -1;
}

/**
 * Returns the client's IPv4
 */
IPAddress HTTPConnection::getClientIP() {
  if (_addrLen > 0 && _sockAddr.sa_family == AF_INET) {
    struct sockaddr_in *sockAddrIn = (struct sockaddr_in *)(&_sockAddr);
    return IPAddress(sockAddrIn->sin_addr.s_addr);
  }
  return IPAddress(0, 0, 0, 0);
}

/**
 * True if the connection is timed out.
 *
 * (Should be checkd in the loop and transition should go to CONNECTION_CLOSE if exceeded)
 */
bool HTTPConnection::isTimeoutExceeded() {
  return _lastTransmissionTS + HTTPS_CONNECTION_TIMEOUT < millis();
}

/**
 * Resets the timeout to allow again the full HTTPS_CONNECTION_TIMEOUT milliseconds
 */
void HTTPConnection::refreshTimeout() {
  _lastTransmissionTS = millis();
}

/**
 * Returns true, if the connection has been closed.
 */
bool HTTPConnection::isClosed() {
  return (_connectionState == STATE_ERROR || _connectionState == STATE_CLOSED);
}

/**
 * Returns true, if the connection has been closed due to error
 */
bool HTTPConnection::isError() {
  return (_connectionState == STATE_ERROR);
}

bool HTTPConnection::isSecure() {
  return false;
}

void HTTPConnection::closeConnection() {
  // TODO: Call an event handler here, maybe?

  if (_connectionState != STATE_ERROR && _connectionState != STATE_CLOSED) {

    // First call to closeConnection - set the timestamp to calculate the timeout later on
    if (_connectionState != STATE_CLOSING) {
      _shutdownTS = millis();
    }

    // Set the connection state to closing. We stay in closing as long as SSL has not been shutdown
    // correctly
    _connectionState = STATE_CLOSING;
  }

  // Tear down the socket
  if (_socket >= 0) {
    HTTPS_LOGI("Connection closed. Socket FID=%d", _socket);
    close(_socket);
    _socket = -1;
    _addrLen = 0;
  }

  if (_connectionState != STATE_ERROR) {
    _connectionState = STATE_CLOSED;
  }

  if (_httpHeaders != NULL) {
    HTTPS_LOGD("Free headers");
    delete _httpHeaders;
    _httpHeaders = NULL;
  }

  if (_wsHandler != nullptr) {
    HTTPS_LOGD("Free WS Handler");
    delete _wsHandler;
    _wsHandler = NULL;
  }
}

/**
 * This method will try to fill up the buffer with data from
 */
int HTTPConnection::updateBuffer() {
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
      if (canReadData()) {

        HTTPS_LOGD("Data on Socket FID=%d", _socket);

        int readReturnCode;

        // The return code of SSL_read means:
        // > 0 : Length of the data that has been read
        // < 0 : Error
        // = 0 : Connection closed
        readReturnCode = readBytesToBuffer(
            // Only after the part of the buffer that has not been processed yet
            (byte*)(_receiveBuffer + sizeof(char) * _bufferUnusedIdx),
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
          HTTPS_LOGI("Client closed connection, FID=%d", _socket);
          // TODO: If we are in state websocket, we might need to do something here
          return 0;
        } else {
          // An error occured
          _connectionState = STATE_ERROR;
          HTTPS_LOGE("An receive error occured, FID=%d", _socket);
          closeConnection();
          return -1;
        }

      } // data pending

    } // buffer can read more
  }
  return 0;
}

bool HTTPConnection::canReadData() {
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

  return FD_ISSET(_socket, &sockfds);
}

size_t HTTPConnection::readBuffer(byte* buffer, size_t length) {
  updateBuffer();
  size_t bufferSize = _bufferUnusedIdx - _bufferProcessed;

  if (length > bufferSize) {
    length = bufferSize;
  }

  // Write until length is reached (either by param of by empty buffer
  for(int i = 0; i < length; i++) {
    buffer[i] = _receiveBuffer[_bufferProcessed++];
  }

  return length;
}

size_t HTTPConnection::pendingBufferSize() {
  updateBuffer();

  return _bufferUnusedIdx - _bufferProcessed + pendingByteCount();
}

size_t HTTPConnection::pendingByteCount() {
  return 0; // FIXME: Add the value of the equivalent function of SSL_pending() here
}

size_t HTTPConnection::writeBuffer(byte* buffer, size_t length) {
  return send(_socket, buffer, length, 0);
}

size_t HTTPConnection::readBytesToBuffer(byte* buffer, size_t length) {
  return recv(_socket, buffer, length, MSG_WAITALL | MSG_DONTWAIT);
}

void HTTPConnection::raiseError(uint16_t code, std::string reason) {
  _connectionState = STATE_ERROR;
  std::string sCode = intToString(code);

  char headers[] = "\r\nConnection: close\r\nContent-Type: text/plain;charset=utf8\r\n\r\n";
  writeBuffer((byte*)"HTTP/1.1 ", 9);
  writeBuffer((byte*)sCode.c_str(), sCode.length());
  writeBuffer((byte*)" ", 1);
  writeBuffer((byte*)(reason.c_str()), reason.length());
  writeBuffer((byte*)headers, strlen(headers));
  writeBuffer((byte*)sCode.c_str(), sCode.length());
  writeBuffer((byte*)" ", 1);
  writeBuffer((byte*)(reason.c_str()), reason.length());
  closeConnection();
}

void HTTPConnection::readLine(int lengthLimit) {
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
          HTTPS_LOGW("Line without \\r\\n (got only \\r). FID=%d", _socket);
          raiseError(400, "Bad Request");
          return;
        }
      }
    } else {
      _parserLine.text += newChar;
      _bufferProcessed += 1;
    }

    // Check that the max request string size is not exceeded
    if (_parserLine.text.length() > lengthLimit) {
      HTTPS_LOGW("Header length exceeded. FID=%d", _socket);
      raiseError(431, "Request Header Fields Too Large");
      return;
    }
  }
}

/**
 * Called by the request to signal that the client has closed the connection
 */
void HTTPConnection::signalClientClose() {
  _clientState = CSTATE_CLOSED;
}

/**
 * Called by the request to signal that an error has occured
 */
void HTTPConnection::signalRequestError() {
  // TODO: Check that no response has been transmitted yet
  raiseError(400, "Bad Request");
}

/**
 * Returns the cache size that should be cached (in the response) to enable keep-alive requests.
 *
 * 0 = no keep alive.
 */
size_t HTTPConnection::getCacheSize() {
  return (_isKeepAlive ? HTTPS_KEEPALIVE_CACHESIZE : 0);
}

void HTTPConnection::loop() {
  // First, update the buffer
  // newByteCount will contain the number of new bytes that have to be processed
  updateBuffer();

  if (_clientState == CSTATE_CLOSED) {
    HTTPS_LOGI("Client closed (FID=%d, cstate=%d)", _socket, _clientState);
  }

  if (_clientState == CSTATE_CLOSED && _bufferProcessed == _bufferUnusedIdx && _connectionState < STATE_HEADERS_FINISHED) {
    closeConnection();
  }

  if (!isClosed() && isTimeoutExceeded()) {
    HTTPS_LOGI("Connection timeout. FID=%d", _socket);
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
          HTTPS_LOGW("Missing space after method");
          raiseError(400, "Bad Request");
          break;
        }
        _httpMethod = _parserLine.text.substr(0, spaceAfterMethodIdx);

        // Find the resource string:
        size_t spaceAfterResourceIdx = _parserLine.text.find(' ', spaceAfterMethodIdx + 1);
        if (spaceAfterResourceIdx == std::string::npos) {
          HTTPS_LOGW("Missing space after resource");
          raiseError(400, "Bad Request");
          break;
        }
        _httpResource = _parserLine.text.substr(spaceAfterMethodIdx + 1, spaceAfterResourceIdx - _httpMethod.length() - 1);

        _parserLine.parsingFinished = false;
        _parserLine.text = "";
        HTTPS_LOGI("Request: %s %s (FID=%d)", _httpMethod.c_str(), _httpResource.c_str(), _socket);
        _connectionState = STATE_REQUEST_FINISHED;
      }

      break;
    case STATE_REQUEST_FINISHED: // Read headers

      while (_bufferProcessed < _bufferUnusedIdx && !isClosed()) {
        readLine(HTTPS_REQUEST_MAX_HEADER_LENGTH);
        if (_parserLine.parsingFinished && _connectionState != STATE_ERROR) {

          if (_parserLine.text.empty()) {
            HTTPS_LOGD("Headers finished, FID=%d", _socket);
            _connectionState = STATE_HEADERS_FINISHED;

            // Break, so that the rest of the body does not get flushed through
            _parserLine.parsingFinished = false;
            _parserLine.text = "";
            break;
          } else {
            int idxColon = _parserLine.text.find(':');
            if ( (idxColon != std::string::npos) && (_parserLine.text[idxColon+1]==' ') ) {
              _httpHeaders->set(new HTTPHeader(
                  _parserLine.text.substr(0, idxColon),
                  _parserLine.text.substr(idxColon+2)
              ));
              HTTPS_LOGD("Header: %s = %s (FID=%d)", _parserLine.text.substr(0, idxColon).c_str(), _parserLine.text.substr(idxColon+2).c_str(), _socket);
            } else {
              HTTPS_LOGW("Malformed request header: %s", _parserLine.text.c_str());
              raiseError(400, "Bad Request");
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
        HTTPS_LOGD("Resolving resource...");
        ResolvedResource resolvedResource;

        // Check which kind of node we need (Websocket or regular)
        bool websocketRequested = checkWebsocket();

        _resResolver->resolveNode(_httpMethod, _httpResource, resolvedResource, websocketRequested ? WEBSOCKET : HANDLER_CALLBACK);

        // Is there any match (may be the defaultNode, if it is configured)
        if (resolvedResource.didMatch()) {
          // Check for client's request to keep-alive if we have a handler function.
          if (resolvedResource.getMatchingNode()->_nodeType == HANDLER_CALLBACK) {
            // Did the client set connection:keep-alive?
            HTTPHeader * connectionHeader = _httpHeaders->get("Connection");
            std::string connectionHeaderValue = "";
            if (connectionHeader != NULL) {
              connectionHeaderValue += connectionHeader->_value;
              std::transform(
                connectionHeaderValue.begin(),
                connectionHeaderValue.end(),
                connectionHeaderValue.begin(),
                [](unsigned char c){ return ::tolower(c); }
              );
            }
            if (std::string("keep-alive").compare(connectionHeaderValue)==0) {
              HTTPS_LOGD("Keep-Alive activated. FID=%d", _socket);
              _isKeepAlive = true;
            } else {
              HTTPS_LOGD("Keep-Alive disabled. FID=%d", _socket);
              _isKeepAlive = false;
            }
          } else {
            _isKeepAlive = false;
          }

          // Create request context
          HTTPRequest req  = HTTPRequest(
            this,
            _httpHeaders,
            resolvedResource.getMatchingNode(),
            _httpMethod,
            resolvedResource.getParams(),
            _httpResource
          );
          HTTPResponse res = HTTPResponse(this);

          // Add default headers to the response
          auto allDefaultHeaders = _defaultHeaders->getAll();
          for(std::vector<HTTPHeader*>::iterator header = allDefaultHeaders->begin(); header != allDefaultHeaders->end(); ++header) {
            res.setHeader((*header)->_name, (*header)->_value);
          }

          // Find the request handler callback
          HTTPSCallbackFunction * resourceCallback;
          if (websocketRequested) {
            // For the websocket, we use the handshake callback defined below
            resourceCallback = &handleWebsocketHandshake;
          } else {
            // For resource nodes, we use the callback defined by the node itself
            resourceCallback = ((ResourceNode*)resolvedResource.getMatchingNode())->_callback;
          }

          // Get the current middleware chain
          auto vecMw = _resResolver->getMiddleware();

          // Anchor of the chain is the actual resource. The call to the handler is bound here
          std::function<void()> next = std::function<void()>(std::bind(resourceCallback, &req, &res));

          // Go back in the middleware chain and glue everything together
          auto itMw = vecMw.rbegin();
          while(itMw != vecMw.rend()) {
            next = std::function<void()>(std::bind((*itMw), &req, &res, next));
            itMw++;
          }

          // We insert the internal validation middleware at the start of the chain:
          next = std::function<void()>(std::bind(&validationMiddleware, &req, &res, next));

          // Call the whole chain
          next();

          // The callback-function should have read all of the request body.
          // However, if it does not, we need to clear the request body now,
          // because otherwise it would be parsed in the next request.
          if (!req.requestComplete()) {
            HTTPS_LOGW("Callback function did not parse full request body");
            req.discardRequestBody();
          }

          // Finally, after the handshake is done, we create the WebsocketHandler and change the internal state.
          if(websocketRequested) {
            _wsHandler = ((WebsocketNode*)resolvedResource.getMatchingNode())->newHandler();
            _wsHandler->initialize(this);  // make websocket with this connection 
            _connectionState = STATE_WEBSOCKET;
          } else {
            // Handling the request is done
            HTTPS_LOGD("Handler function done, request complete");

            // Now we need to check if we can use keep-alive to reuse the SSL connection
            // However, if the client did not set content-size or defined connection: close,
            // we have no chance to do so.
            // Also, the programmer may have explicitly set Connection: close for the response.
            std::string hConnection = res.getHeader("Connection");
            if (hConnection == "close") {
              _isKeepAlive = false;
            }
            if (!_isKeepAlive) {
              // No KeepAlive -> We are done. Transition to next state.
              if (!isClosed()) {
                res.finalize();
                _connectionState = STATE_BODY_FINISHED;
              }
            } else {
              if (res.isResponseBuffered()) {
                // If the response could be buffered:
                res.setHeader("Connection", "keep-alive");
                res.finalize();
                if (_clientState != CSTATE_CLOSED) {
                  // Refresh the timeout for the new request
                  refreshTimeout();
                  // Reset headers for the new connection
                  _httpHeaders->clearAll();
                  // Go back to initial state
                  _connectionState = STATE_INITIAL;
                }
              }
              // The response could not be buffered or the client has closed:
              if (!isClosed() && _connectionState!=STATE_INITIAL) {
                _connectionState = STATE_BODY_FINISHED;
              }
            }
          }
        } else {
          // No match (no default route configured, nothing does match)
          HTTPS_LOGW("Could not find a matching resource");
          raiseError(404, "Not Found");
        }

      }
      break;
    case STATE_BODY_FINISHED: // Request is complete
      closeConnection();
      break;
    case STATE_CLOSING: // As long as we are in closing state, we call closeConnection() again and wait for it to finish or timeout
      closeConnection();
      break;
    case STATE_WEBSOCKET: // Do handling of the websocket
      refreshTimeout();  // don't timeout websocket connection
      if(pendingBufferSize() > 0) {
        HTTPS_LOGD("Calling WS handler, FID=%d", _socket);
        _wsHandler->loop();
      }

      // If the client closed the connection unexpectedly
      if (_clientState == CSTATE_CLOSED) {
        HTTPS_LOGI("WS lost client, calling onClose, FID=%d", _socket);
        _wsHandler->onClose();
      }

      // If the handler has terminated the connection, clean up and close the socket too
      if (_wsHandler->closed() || _clientState == CSTATE_CLOSED) {
        HTTPS_LOGI("WS closed, freeing Handler, FID=%d", _socket);
        delete _wsHandler;
        _wsHandler = nullptr;
        _connectionState = STATE_CLOSING;
      }
      break;
    default:;
    }
  }

}


bool HTTPConnection::checkWebsocket() {
  if(_httpMethod == "GET" &&
     !_httpHeaders->getValue("Host").empty() &&
      _httpHeaders->getValue("Upgrade") == "websocket" &&
      _httpHeaders->getValue("Connection").find("Upgrade") != std::string::npos &&
     !_httpHeaders->getValue("Sec-WebSocket-Key").empty() &&
      _httpHeaders->getValue("Sec-WebSocket-Version") == "13") {

      HTTPS_LOGI("Upgrading to WS, FID=%d", _socket);
      return true;
  } else
      return false;
}

/**
 * Middleware function that handles the validation of parameters
 */
void validationMiddleware(HTTPRequest * req, HTTPResponse * res, std::function<void()> next) {
  bool valid = true;
  // Get the matched node
  HTTPNode * node = req->getResolvedNode();
  // Get the parameters
  ResourceParameters * params = req->getParams();

  // Iterate over the validators and run them
  std::vector<HTTPValidator*> * validators = node->getValidators();
  for(std::vector<HTTPValidator*>::iterator validator = validators->begin(); valid && validator != validators->end(); ++validator) {
    std::string param;
    if (params->getPathParameter((*validator)->_idx, param)) {
      valid = ((*validator)->_validatorFunction)(param);
    } else {
      valid = false;
    }
  }

  if (valid) {
    next();
  } else {
    res->setStatusCode(400);
    res->setStatusText("Bad Request");
    res->print("400 Bad Request");
  }
}

/**
 * Handler function for the websocket handshake. Will be used by HTTPConnection if a websocket is detected
 */
void handleWebsocketHandshake(HTTPRequest * req, HTTPResponse * res) {
  res->setStatusCode(101);
  res->setStatusText("Switching Protocols");
  res->setHeader("Upgrade", "websocket");
  res->setHeader("Connection", "Upgrade");
  res->setHeader("Sec-WebSocket-Accept", websocketKeyResponseHash(req->getHeader("Sec-WebSocket-Key")));
  res->print("");
}

/**
 * Function used to compute the value of the Sec-WebSocket-Accept during Websocket handshake
 */
std::string websocketKeyResponseHash(std::string const &key) {
  std::string newKey = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  uint8_t shaData[HTTPS_SHA1_LENGTH];
  esp_sha(SHA1, (uint8_t*)newKey.data(), newKey.length(), shaData);

  // Get output size required for base64 representation
  size_t b64BufferSize = 0;
  mbedtls_base64_encode(nullptr, 0, &b64BufferSize, (const unsigned char*)shaData, HTTPS_SHA1_LENGTH);

  // Do the real encoding
  unsigned char bufferOut[b64BufferSize];
  size_t bytesEncoded = 0;
  int res = mbedtls_base64_encode(
    bufferOut,
    b64BufferSize,
    &bytesEncoded,
    (const unsigned char*)shaData,
    HTTPS_SHA1_LENGTH
  );
  
  // Check result and return the encoded string
  if (res != 0) {
    return std::string();
  }
  return std::string((char*)bufferOut, bytesEncoded);
} // WebsocketKeyResponseHash

} /* namespace httpsserver */
