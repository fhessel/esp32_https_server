/*
 * HTTPSServerConstants.hpp
 *
 *  Created on: Dec 14, 2017
 *      Author: frank
 */

#ifndef HTTPS_HTTPSSERVERCONSTANTS_HPP_
#define HTTPS_HTTPSSERVERCONSTANTS_HPP_

#include "Arduino.h"

// Debug Code. Undefine it to disable debugging output
#define HTTPS_DLOG(X)      Serial.print("HTTPSServer->debug: ");Serial.println(X);
#define HTTPS_DLOGHEX(X,Y) Serial.print("HTTPSServer->debug: ");Serial.print(X);Serial.print(" 0x");Serial.println(Y, HEX);

// The following lines define limits of the protocol. Exceeding these limits will lead to a 500 error

// Maximum of header lines that are parsed
#define HTTPS_REQUEST_MAX_HEADERS               20

// Maximum length of the request line (GET /... HTTP/1.1)
#define HTTPS_REQUEST_MAX_REQUEST_LENGTH       128

// Maximum length of a header line (including name and value)
#define HTTPS_REQUEST_MAX_HEADER_LENGTH        384

// Chunk size used for reading data from the ssl-enabled socket
#define HTTPS_CONNECTION_DATA_CHUNK_SIZE       512

#endif /* HTTPS_HTTPSSERVERCONSTANTS_HPP_ */
