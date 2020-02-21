#ifndef SRC_WEBSOCKETHANDLER_HPP_
#define SRC_WEBSOCKETHANDLER_HPP_

#include <Arduino.h>
#include <lwip/def.h>

#include <string>
#undef min
#undef max

#include <sstream>

#include "HTTPSServerConstants.hpp"
#include "ConnectionContext.hpp"
#include "WebsocketInputStreambuf.hpp"

namespace httpsserver {

// Structure definition for the WebSocket frame.
struct WebsocketFrame
{
  // Byte 0
  uint8_t opCode : 4; // [7:4]
  uint8_t rsv3 : 1;   // [3]
  uint8_t rsv2 : 1;   // [2]
  uint8_t rsv1 : 1;   // [1]
  uint8_t fin : 1;    // [0]

  // Byte 1
  uint8_t len : 7;  // [7:1]
  uint8_t mask : 1; // [0]
};

class WebsocketHandler
{
public:
  // WebSocket op codes as found in a WebSocket frame.
  static const int OPCODE_CONTINUE = 0x00;
  static const int OPCODE_TEXT = 0x01;
  static const int OPCODE_BINARY = 0x02;
  static const int OPCODE_CLOSE = 0x08;
  static const int OPCODE_PING = 0x09;
  static const int OPCODE_PONG = 0x0a;

  static const uint16_t CLOSE_NORMAL_CLOSURE = 1000;
  static const uint16_t CLOSE_GOING_AWAY = 1001;
  static const uint16_t CLOSE_PROTOCOL_ERROR = 1002;
  static const uint16_t CLOSE_CANNOT_ACCEPT = 1003;
  static const uint16_t CLOSE_NO_STATUS_CODE = 1005;
  static const uint16_t CLOSE_CLOSED_ABNORMALLY = 1006;
  static const uint16_t CLOSE_NOT_CONSISTENT = 1007;
  static const uint16_t CLOSE_VIOLATED_POLICY = 1008;
  static const uint16_t CLOSE_TOO_BIG = 1009;
  static const uint16_t CLOSE_NO_EXTENSION = 1010;
  static const uint16_t CLOSE_UNEXPECTED_CONDITION = 1011;
  static const uint16_t CLOSE_SERVICE_RESTART = 1012;
  static const uint16_t CLOSE_TRY_AGAIN_LATER = 1013;
  static const uint16_t CLOSE_TLS_HANDSHAKE_FAILURE = 1015;

  static const uint8_t SEND_TYPE_BINARY = 0x01;
  static const uint8_t SEND_TYPE_TEXT = 0x02;

  WebsocketHandler();
  virtual ~WebsocketHandler();
  virtual void onClose();
  virtual void onMessage(WebsocketInputStreambuf *pWebsocketInputStreambuf);
  virtual void onError(std::string error);

  void close(uint16_t status = CLOSE_NORMAL_CLOSURE, std::string message = "");
  void send(std::string data, uint8_t sendType = SEND_TYPE_BINARY);
  void send(uint8_t *data, uint16_t length, uint8_t sendType = SEND_TYPE_BINARY);
  bool closed();

  void loop();
  void initialize(ConnectionContext * con);

private:
  int read();

  ConnectionContext * _con;
  bool _receivedClose; // True when we have received a close request.
  bool _sentClose; // True when we have sent a close request.
};

}

#endif
