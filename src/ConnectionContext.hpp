#ifndef SRC_CONNECTIONCONTEXT_HPP_
#define SRC_CONNECTIONCONTEXT_HPP_

#include <Arduino.h>

// Required for SSL
#include "openssl/ssl.h"
#undef read

namespace httpsserver {

class WebsocketHandler;

class ConnectionContext {
public:
  ConnectionContext();
  virtual ~ConnectionContext();

  virtual void signalRequestError() = 0;
  virtual void signalClientClose() = 0;
  virtual size_t getCacheSize() = 0;

  virtual size_t readBuffer(byte* buffer, size_t length) = 0;
  virtual size_t pendingBufferSize() = 0;

  virtual size_t writeBuffer(byte* buffer, size_t length) = 0;

  virtual bool isSecure() = 0;
  virtual void setWebsocketHandler(WebsocketHandler *wsHandler);

  WebsocketHandler * _wsHandler;
};

} /* namespace httpsserver */

#endif /* SRC_CONNECTIONCONTEXT_HPP_ */
