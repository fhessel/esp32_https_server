// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _https_server_H_
#define _https_server_H_

// Arduino libraries
#include "Arduino.h"

// For multitasking
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

// We will use wifi
#include <WiFi.h>

// Copy data/wifi/wifi.example.h to /data/wifi/wifi.h and change the ssid and psk
#include "data/wifi/wifi.h"

// Run tools/cert/create_cert.sh to create a CA and issue a certificate that will
// be stored in the data/cert/ header files
#include "data/cert/cert.h"
#include "data/cert/private_key.h"

// The favicon (binary)
#include "data/favicon.h"

// Inlcudes for setting up the server
#include "src/HTTPSServer.hpp"

// Includes to define request handler callbacks
#include "src/HTTPRequest.hpp"
#include "src/HTTPResponse.hpp"

// The server loop will be configured as a separate task, so the server will run
// independently from all other code.
// As an alternative, it's possible to call the HTTPServer::loop() function in the
// main loop after setting up the server in setup().
// The loop function triggers all connection handling etc. in the background
void serverTask(void * params);

#endif /* _https_server_H_ */
