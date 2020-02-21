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
 *  - Show simple page on web server root
 *  - 404 for everything else
 * The server will be run in a separate task, so that you can do your own stuff
 * in the loop() function.
 * Everything else is just like the Static-Page example
 */

// TODO: Configure your WiFi here
#define WIFI_SSID "<your ssid goes here>"
#define WIFI_PSK  "<your pre-shared key goes here>"

/** Check if we have multiple cores */
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

// Include certificate data (see note above)
#include "cert.h"
#include "private_key.h"

// We will use wifi
#include <WiFi.h>

// Includes for the server
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

// The HTTPS Server comes in a separate namespace. For easier use, include it here.
using namespace httpsserver;

// Create an SSL certificate object from the files included above
SSLCert cert = SSLCert(
  example_crt_DER, example_crt_DER_len,
  example_key_DER, example_key_DER_len
);

// Create an SSL-enabled server that uses the certificate
HTTPSServer secureServer = HTTPSServer(&cert);

// Declare some handler functions for the various URLs on the server
void handleRoot(HTTPRequest * req, HTTPResponse * res);
void handle404(HTTPRequest * req, HTTPResponse * res);

// We declare a function that will be the entry-point for the task that is going to be
// created.
void serverTask(void *params);

void setup() {
  // For logging
  Serial.begin(115200);

  // Connect to WiFi
  Serial.println("Setting up WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected. IP=");
  Serial.println(WiFi.localIP());

  // Setup the server as a separate task.
  Serial.println("Creating server task... ");
  // We pass:
  // serverTask - the function that should be run as separate task
  // "https443" - a name for the task (mainly used for logging)
  // 6144       - stack size in byte. If you want up to four clients, you should
  //              not go below 6kB. If your stack is too small, you will encounter
  //              Panic and stack canary exceptions, usually during the call to
  //              SSL_accept.
  xTaskCreatePinnedToCore(serverTask, "https443", 6144, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
}

void loop() {
  Serial.println("loop()");
  delay(5000);
}

void serverTask(void *params) {
  // In the separate task we first do everything that we would have done in the
  // setup() function, if we would run the server synchronously.

  // Note: The second task has its own stack, so you need to think about where
  // you create the server's resources and how to make sure that the server
  // can access everything it needs to access. Also make sure that concurrent
  // access is no problem in your sketch or implement countermeasures like locks
  // or mutexes.

  // Create nodes
  ResourceNode * nodeRoot    = new ResourceNode("/", "GET", &handleRoot);
  ResourceNode * node404     = new ResourceNode("", "GET", &handle404);

  // Add nodes to the server
  secureServer.registerNode(nodeRoot);
  secureServer.setDefaultNode(node404);

  Serial.println("Starting server...");
  secureServer.start();
  if (secureServer.isRunning()) {
    Serial.println("Server ready.");

    // "loop()" function of the separate task
    while(true) {
      // This call will let the server do its work
      secureServer.loop();

      // Other code would go here...
      delay(1);
    }
  }
}

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
  res->println("</body>");
  res->println("</html>");
}

void handle404(HTTPRequest * req, HTTPResponse * res) {
  // Discard request body, if we received any
  // We do this, as this is the default node and may also server POST/PUT requests
  req->discardRequestBody();

  // Set the response status
  res->setStatusCode(404);
  res->setStatusText("Not Found");

  // Set content type of the response
  res->setHeader("Content-Type", "text/html");

  // Write a tiny HTTP page
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Not Found</title></head>");
  res->println("<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>");
  res->println("</html>");
}
