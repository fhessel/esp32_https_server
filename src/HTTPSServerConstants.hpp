#ifndef SRC_HTTPSSERVERCONSTANTS_HPP_
#define SRC_HTTPSSERVERCONSTANTS_HPP_

#include "Arduino.h"

// Debug Code. Undefine it to disable debugging output
#define HTTPS_DLOG(X)      Serial.print(millis());Serial.print(" HTTPSServer->debug: ");Serial.println(X);
#define HTTPS_DLOGHEX(X,Y) Serial.print(millis());Serial.print(" HTTPSServer->debug: ");Serial.print(X);Serial.print(" 0x");Serial.println(Y, HEX);
#define HTTPS_DLOGINT(X,Y) Serial.print(millis());Serial.print(" HTTPSServer->debug: ");Serial.print(X);Serial.println(Y);

// The following lines define limits of the protocol. Exceeding these limits will lead to a 500 error

// Maximum of header lines that are parsed
#define HTTPS_REQUEST_MAX_HEADERS               20

// Maximum length of the request line (GET /... HTTP/1.1)
#define HTTPS_REQUEST_MAX_REQUEST_LENGTH       128

// Maximum length of a header line (including name and value)
#define HTTPS_REQUEST_MAX_HEADER_LENGTH        384

// Chunk size used for reading data from the ssl-enabled socket
#define HTTPS_CONNECTION_DATA_CHUNK_SIZE       512

// Size (in bytes) of the Connection:keep-alive Cache (we need to be able to
// store-and-forward the response to calculate the content-size)
#define HTTPS_KEEPALIVE_CACHESIZE              1400

// Timeout for an HTTPS connection without any transmission
#define HTTPS_CONNECTION_TIMEOUT               20000

// Timeout used to wait for shutdown of SSL connection (ms)
// (time for the client to return notify close flag) - without it, truncation attacks might be possible
#define HTTPS_SHUTDOWN_TIMEOUT                 5000

// Length of a SHA1 hash
#define HTTPS_SHA1_LENGTH                      20

#endif /* SRC_HTTPSSERVERCONSTANTS_HPP_ */
