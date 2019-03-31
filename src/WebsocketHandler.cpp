#include "WebsocketHandler.hpp"

namespace httpsserver {

/**
 * @brief Dump the content of the WebSocket frame for debugging.
 * @param [in] frame The frame to dump.
 */
static void dumpFrame(WebsocketFrame frame) {
  std::ostringstream oss;
  oss << "Fin: " << (int)frame.fin << ", OpCode: " << (int)frame.opCode;
  switch(frame.opCode) {
    case WebsocketHandler::OPCODE_BINARY: {
      oss << " BINARY";
      break;
    }
    case WebsocketHandler::OPCODE_CONTINUE: {
      oss << " CONTINUE";
      break;
    }
    case WebsocketHandler::OPCODE_CLOSE: {
      oss << " CLOSE";
      break;
    }
    case WebsocketHandler::OPCODE_PING: {
      oss << " PING";
      break;
    }
    case WebsocketHandler::OPCODE_PONG: {
      oss << " PONG";
      break;
    }
    case WebsocketHandler::OPCODE_TEXT: {
      oss << " TEXT";
      break;
    }
    default: {
      oss << " Unknown";
      break;
    }
  }
  oss << ", Mask: " << (int)frame.mask << ", len: " << (int)frame.len;
  std::string s = "[Frm] " + oss.str();
  HTTPS_DLOG(s.c_str());
} // dumpFrame

WebsocketHandler::WebsocketHandler() {
  _con = nullptr;
  _receivedClose = false;
  _sentClose = false;
}

WebsocketHandler::~WebsocketHandler() {

} // ~WebSocketHandler()


/**
* @brief The default onClose handler.
* If no over-riding handler is provided for the "close" event, this method is called.
*/
void WebsocketHandler::onClose() {
  HTTPS_DLOG("[   ] WebsocketHandler close()");
} // onClose

/**
* @brief The default onData handler.
* If no over-riding handler is provided for the "message" event, this method is called.
* A particularly useful pattern for using onMessage is:
* ```
* std::stringstream buffer;
* buffer << pWebSocketInputRecordStreambuf;
* ```
* This will read the whole message into the string stream.
*/
void WebsocketHandler::onMessage(WebsocketInputStreambuf* pWebsocketInputStreambuf) { //, Websocket *pWebSocket) {
  HTTPS_DLOG("[   ] WebsocketHandler onMessage()");
} // onData


/**
* @brief The default onError handler.
* If no over-riding handler is provided for the "error" event, this method is called.
*/
void WebsocketHandler::onError(std::string error) {
  HTTPS_DLOG("[   ] WebsocketHandler onError()");
} // onError

void WebsocketHandler::initialize(ConnectionContext * con) {
  _con = con;
}

void WebsocketHandler::loop() {
  if(read() < 0) {
    close();
  }
}

int WebsocketHandler::read() {
  WebsocketFrame frame;
  int length = _con->readBuffer((uint8_t*)&frame, sizeof(frame));
  HTTPS_DLOGHEX("[   ] read bytes:", length);
  if(length == 0) 
    return 0;
  else if (length != sizeof(frame)) {
    HTTPS_DLOG("[ERR] Websocket read error");
    //_con->closeConnection();
    return -1;
  }
  dumpFrame(frame);

  // The following section parses the WebSocket frame.
  uint32_t payloadLen = 0;
  uint8_t  mask[4];
  if (frame.len < 126) {
    payloadLen = frame.len;
  } else if (frame.len == 126) {
    uint16_t tempLen;
    _con->readBuffer((uint8_t*)&tempLen, sizeof(tempLen));
    payloadLen = ntohs(tempLen);
  } else if (frame.len == 127) {
    uint64_t tempLen;
    _con->readBuffer((uint8_t*)&tempLen, sizeof(tempLen));
    payloadLen = ntohl((uint32_t)tempLen);
  }
  if (frame.mask == 1) {
    _con->readBuffer(mask, sizeof(mask));
  }

  if (payloadLen == 0) {
    HTTPS_DLOG("[WRN] Web socket payload is not present");
  } else {
    HTTPS_DLOGHEX("[   ] Web socket payload, length=%d:", payloadLen);
  }

  switch(frame.opCode) {
    case OPCODE_TEXT:
    case OPCODE_BINARY: {
      HTTPS_DLOG("[   ] Creating Streambuf");
      WebsocketInputStreambuf streambuf(_con, payloadLen, frame.mask==1?mask:nullptr);
      HTTPS_DLOG("[   ] Calling onMessage");
      onMessage(&streambuf);
      HTTPS_DLOG("[   ] Discarding Streambuf");
      streambuf.discard();
      break;
    }

    case OPCODE_CLOSE: {  // If the WebSocket operation code is close then we are closing the connection.
      _receivedClose = true;
      onClose();
      //close();                // Close the websocket.
      return -1;
      break;
    }

    case OPCODE_CONTINUE: {
      break;
    }

    case OPCODE_PING: {
      break;
    }

    case OPCODE_PONG: {
      break;
    }

    default: {
        HTTPS_DLOGHEX("[   ] WebSocketReader: Unknown opcode: ", frame.opCode);
      break;
    }
  } // Switch opCode
  return 0;
}  // Websocket::read

/**
 * @brief Close the Web socket
 * @param [in] status The code passed in the close request.
 * @param [in] message A clarification message on the close request.
 */
void WebsocketHandler::close(uint16_t status, std::string message) {
  HTTPS_DLOG("[   ] >> Websocket close()");

  _sentClose = true;              // Flag that we have sent a close request.

  WebsocketFrame frame;           // Build the web socket frame indicating a close request.
  frame.fin    = 1;
  frame.rsv1   = 0;
  frame.rsv2   = 0;
  frame.rsv3   = 0;
  frame.opCode = OPCODE_CLOSE;
  frame.mask   = 0;
  frame.len    = message.length() + 2;
  int rc = _con->writeBuffer((uint8_t *)&frame, sizeof(frame));

  if (rc > 0) {
    rc = _con->writeBuffer((byte *) &status, 2);
  }

  if (rc > 0) {
    _con->writeBuffer((byte *) message.data(), message.length());
  }
} // Websocket::close

/**
 * @brief Send data down the web socket
 * See the WebSocket spec (RFC6455) section "6.1 Sending Data".
 * We build a WebSocket frame, send the frame followed by the data.
 * @param [in] data The data to send down the WebSocket.
 * @param [in] sendType The type of payload.  Either SEND_TYPE_TEXT or SEND_TYPE_BINARY.
 */
void WebsocketHandler::send(std::string data, uint8_t sendType) {
  HTTPS_DLOGHEX(">> Websocket.send: Length: ", data.length());
  WebsocketFrame frame;
  frame.fin    = 1;
  frame.rsv1   = 0;
  frame.rsv2   = 0;
  frame.rsv3   = 0;
  frame.opCode = sendType==SEND_TYPE_TEXT?OPCODE_TEXT:OPCODE_BINARY;
  frame.mask   = 0;
  if (data.length() < 126) {
    frame.len = data.length();
    _con->writeBuffer((uint8_t *)&frame, sizeof(frame));
  } else {
    frame.len = 126;
    _con->writeBuffer((uint8_t *)&frame, sizeof(frame));
    uint16_t net_len = htons((uint16_t)data.length());
    _con->writeBuffer((uint8_t *)&net_len, sizeof(uint16_t));  // Convert to network byte order from host byte order
  }
  _con->writeBuffer((uint8_t*)data.data(), data.length());
  HTTPS_DLOG("[   ] << Websocket.send");
} // Websocket::send


/**
 * @brief Send data down the web socket
 * See the WebSocket spec (RFC6455) section "6.1 Sending Data".
 * We build a WebSocket frame, send the frame followed by the data.
 * @param [in] data The data to send down the WebSocket.
 * @param [in] sendType The type of payload.  Either SEND_TYPE_TEXT or SEND_TYPE_BINARY.
 */
void WebsocketHandler::send(uint8_t* data, uint16_t length, uint8_t sendType) {
  HTTPS_DLOGHEX("[   ] >> Websocket.send: Length: ", length);
  WebsocketFrame frame;
  frame.fin    = 1;
  frame.rsv1   = 0;
  frame.rsv2   = 0;
  frame.rsv3   = 0;
  frame.opCode = sendType==SEND_TYPE_TEXT?OPCODE_TEXT:OPCODE_BINARY;
  frame.mask   = 0;
  if (length < 126) {
    frame.len = length;
    _con->writeBuffer((uint8_t *)&frame, sizeof(frame));
  } else {
    frame.len = 126;
    _con->writeBuffer((uint8_t *)&frame, sizeof(frame));
    uint16_t net_len = htons(length);
    _con->writeBuffer((uint8_t *) net_len, sizeof(uint16_t));  // Convert to network byte order from host byte order
  }
  _con->writeBuffer(data, length);
  HTTPS_DLOG("[   ] << Websocket.send");
}  // Websocket::send

/**
 * Returns true if the connection has been closed, either by client or server
 */
bool WebsocketHandler::closed() {
  return _receivedClose || _sentClose;
}

}
