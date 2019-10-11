/**
 * Example for the ESP32 HTTP(S) Webserver
 *
 * IMPORTANT NOTE:
 * To run this script, your need to
 *  1) Enter your WiFi SSID and PSK below this comment
 *  2) Make sure to have certificate data available. You will find a
 *     shell script and instructions to do so in the library folder
 *     under extras/
 *
 * This script will install an HTTPS Server on your ESP32 with the following
 * functionalities:
 *  - Use of TLS Tickets (RFC 5077)
 *  - Use of server workers in separate FreeRTOS tasks
 *  - Show simple page on web server root
 *  - Paralell serving of multiple images (cats.html)
 *  
 */

/**
 * NOTE: You need to upload the data directory to SPIFFS
 * 
 * Cat images in data folder were taken from: 
 * https://unsplash.com/search/photos/cat
 * License: https://unsplash.com/license
 */

// TODO: Configure your WiFi here
#define WIFI_SSID "<your ssid goes here>"
#define WIFI_PSK  "<your pre-shared key goes here>"

// Adjust these for available memory
#define MAX_CONNECTIONS 2
#define NUM_WORKERS     2

//Delay image sending, milisecods between 256 bytes blocks.
//Can be used to visualize parallel connection handling
#define IMAGE_SLOWDOWN  10

// Include certificate data (see note above)
#include "cert.h"
#include "private_key.h"

// We will use wifi
#include <WiFi.h>

// And images stored in SPIFFS
#include "FS.h"
#include "SPIFFS.h"

// Includes for the server
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <cJSON.h>

// The HTTPS Server comes in a separate namespace. For easier use, include it here.
using namespace httpsserver;

// Create an SSL certificate object from the files included above
SSLCert cert = SSLCert(
  example_crt_DER, example_crt_DER_len,
  example_key_DER, example_key_DER_len
);

// Create an TLS-enabled server that uses the certificate
HTTPSServer secureServer = HTTPSServer(&cert, 443, MAX_CONNECTIONS);

// Declare some handler functions for the various URLs on the server
void handleRoot(HTTPRequest * req, HTTPResponse * res);
void handleCatIndex(HTTPRequest * req, HTTPResponse * res);
void handleDefault(HTTPRequest * req, HTTPResponse * res);

void setup() {
  // For logging
  Serial.begin(115200);

  // Check SPIFFS data
  if (!SPIFFS.begin(false)) {
    Serial.println("Please upload SPIFFS data for this sketch.");
    while(1);
  }

  // Connect to WiFi
  Serial.println("Setting up WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected. IP=");
  Serial.println(WiFi.localIP());

  Serial.println("Memory before server start:");
  Serial.print("  heap size = "); Serial.println(heap_caps_get_free_size(MALLOC_CAP_8BIT));
  Serial.print("  largest free block = "); Serial.println(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));

  // Add nodes to the server
  secureServer.registerNode(new ResourceNode("/", "GET", &handleRoot));
  secureServer.registerNode(new ResourceNode("/cats.html", "GET", &handleCatIndex));
  secureServer.setDefaultNode(new ResourceNode("", "GET", &handleDefault));
        
  Serial.println("Starting server...");

  // We want to use RFC5077 TLS ticket for faster 
  // TLS connenction negotiation once one connection
  // was established. Tickets are by default valid 
  // for one day
  // Must be configure before you start the server
  secureServer.enableTLSTickets();

  // Enable two FreeRTOS task that will independedntly 
  // handle all server functions and resource callbacks 
  // Must be configure before you start the server
  secureServer.enableWorkers(NUM_WORKERS);

  // And finally start the HTTPS server
  secureServer.start();

  // We are done. You are free to use the loop() function
  // without minding the server... that is if you still have 
  // RAM available...
  
  if (secureServer.isRunning()) {
    Serial.println("Server is running.");
  }

  Serial.println("Memory after server start:");
  Serial.print("  heap size = "); Serial.println(heap_caps_get_free_size(MALLOC_CAP_8BIT));
  Serial.print("  largest free block = "); Serial.println(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
}

void loop() {
  Serial.println("main loop()");
  delay(5000);
}

// HTTPS Server handlers 

void handleRoot(HTTPRequest * req, HTTPResponse * res) {
  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");

  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Hello World!</title></head>");
  res->println("<body>");
  res->println("<h1>Hello World!</h1>");
  res->print("<p>Your server is running for ");
  // A bit of dynamic data: Show the uptime
  res->print((int)(millis()/1000), DEC);
  res->println(" seconds.</p>");
  res->print("<p>Task servicing this connection is: ");
  res->print((uint32_t)xTaskGetCurrentTaskHandle(), HEX);
  res->println("</p>");
  res->println("<p>Here you can find some random <a href='cats.html'>cats</a></p>");
  res->println("</body>");
  res->println("</html>");
}

/**
 * Generate page with links to images in SPIFFS:/cats directory
 */
void handleCatIndex(HTTPRequest * req, HTTPResponse * res) {
  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");

  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Random cats</title></head>");
  res->println("<body>");
  File dir = SPIFFS.open("/cats");
  if (dir.isDirectory()) {
    File file = dir.openNextFile();
    while (file) {
      String filename = file.name();
      if (filename.endsWith(".jpg")) {
        res->print("<img src='");
        res->print(file.name());
        res->println("' style='width:20%;height:20%'>");
      }
      file = dir.openNextFile();
    }
  }
  res->println("</body>");
  res->println("</html>");
}

// Default handler looks for matching file in SPIFFS 
// and sends it, otherwise retuns 404
void handleDefault(HTTPRequest * req, HTTPResponse * res) {  
  // Discard request body, if we received any
  req->discardRequestBody();

  // Find the file in SPIFFS
  String filename = String(req->getRequestString().c_str());
  if (!SPIFFS.exists(filename)) {
    // File doesn't exist, return 404
    res->setStatusCode(404);
    res->setStatusText("Not found");
    // Write a tiny HTTP page
    res->setHeader("Content-Type", "text/html");
    res->println("<!DOCTYPE html>");
    res->println("<html>");
    res->println("<head><title>Not Found</title></head>");
    res->println("<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>");
    res->println("</html>");
    return;
  }
  File file = SPIFFS.open(filename);

  // Set headers, "Content-Length" is important!
  res->setHeader("Content-Length", httpsserver::intToString(file.size()));
  res->setHeader("Content-Type", "image/jpg");
  // Informational only, if you look at developer console in your browser
  char taskHandle[11];
  sprintf(taskHandle, "0x%08x", (uint32_t)xTaskGetCurrentTaskHandle());
  res->setHeader("X-Task-ID", taskHandle);

  // Allocate buffer in the task stack as this may run in parallel
  uint8_t buffer[256];
  // Send file contents
  size_t length = 0;
  do {
    length = file.read(buffer, sizeof(buffer));
    res->write(buffer, length);
    #if IMAGE_SLOWDOWN > 0
    delay(IMAGE_SLOWDOWN); 
    #endif
  } while (length > 0);
  file.close();
}
