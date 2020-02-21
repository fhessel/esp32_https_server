#include "ConnectionContext.hpp"

namespace httpsserver {

ConnectionContext::ConnectionContext() {

}

ConnectionContext::~ConnectionContext() {
	
}

void ConnectionContext::setWebsocketHandler(WebsocketHandler *wsHandler) {
  _wsHandler = wsHandler;
}

} /* namespace httpsserver */
