#ifndef SRC_HTTPSCONNECTION_HPP_
#define SRC_HTTPSCONNECTION_HPP_

#include <Arduino.h>

#include <string>

// Required for SSL
//#include "openssl/ssl.h"
//#undef read
#include <esp_tls.h>

// Required for sockets
#include "lwip/netdb.h"
#undef read
#include "lwip/sockets.h"

#include "HTTPSServerConstants.hpp"
#include "HTTPConnection.hpp"
#include "HTTPHeaders.hpp"
#include "HTTPHeader.hpp"
#include "ResourceResolver.hpp"
#include "ResolvedResource.hpp"
#include "ResourceNode.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

namespace httpsserver {

/**
 * \brief Connection class for an open TLS-enabled connection to an HTTPSServer
 */
class HTTPSConnection : public HTTPConnection {
public:
  HTTPSConnection(ResourceResolver * resResolver);
  virtual ~HTTPSConnection();

  virtual int initialize(int serverSocketID,esp_tls_cfg_server_t * cfgSrv, HTTPHeaders *defaultHeaders);
  virtual void closeConnection();
  virtual bool isSecure();

protected:
  friend class HTTPRequest;
  friend class HTTPResponse;

  virtual size_t readBytesToBuffer(byte* buffer, size_t length);
  virtual size_t pendingByteCount();
  virtual bool canReadData();
  virtual size_t writeBuffer(byte* buffer, size_t length);

private:
  // SSL context for this connection
  esp_tls_t * _ssl;
  esp_tls_cfg_server_t * _cfg;
};

} /* namespace httpsserver */

#endif /* SRC_HTTPSCONNECTION_HPP_ */
