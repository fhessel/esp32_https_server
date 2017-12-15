/*
 * ConnectionContext.hpp
 *
 *  Created on: Dec 15, 2017
 *      Author: frank
 */

#ifndef HTTPS_CONNECTIONCONTEXT_HPP_
#define HTTPS_CONNECTIONCONTEXT_HPP_

#include <Arduino.h>

// Required for SSL
#include "openssl/ssl.h"
#undef read

namespace httpsserver {

class ConnectionContext {
public:
	ConnectionContext();
	virtual ~ConnectionContext();

	virtual SSL* ssl() = 0;
};

} /* namespace httpsserver */

#endif /* HTTPS_CONNECTIONCONTEXT_HPP_ */
