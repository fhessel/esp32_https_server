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
 * This script will install an HTTP Server on port 80 and an HTTPS server
 * on port 443 of your ESP32 with the following functionalities:
 *  - Show simple page on web server root
 *  - 404 for everything else
 *
 * The comments in this script focus on making both protocols available,
 * for setting up the server itself, see Static-Page.
 */

// TODO: Configure your WiFi here
#define WIFI_SSID "<your ssid goes here>"
#define WIFI_PSK  "<your pre-shared key goes here>"

// Include certificate data (see note above)
#include "cert.h"
#include "private_key.h"

// We will use wifi
#include <WiFi.h>

// Includes for the server
// Note: We include HTTPServer and HTTPSServer
#include <HTTPSServer.hpp>
#include <HTTPServer.hpp>
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

// First, we create the HTTPSServer with the certificate created above
HTTPSServer secureServer = HTTPSServer(&cert);

// Additionally, we create an HTTPServer for unencrypted traffic
HTTPServer insecureServer = HTTPServer();

// Declare some handler functions for the various URLs on the server
void handleRoot(HTTPRequest * req, HTTPResponse * res);
void handle404(HTTPRequest * req, HTTPResponse * res);

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

  // For every resource available on the server, we need to create a ResourceNode
  // The ResourceNode links URL and HTTP method to a handler function
  ResourceNode * nodeRoot = new ResourceNode("/", "GET", &handleRoot);
  ResourceNode * node404  = new ResourceNode("", "GET", &handle404);

  // Add the root node to the servers. We can use the same ResourceNode on multiple
  // servers (you could also run multiple HTTPS servers)
  secureServer.registerNode(nodeRoot);
  insecureServer.registerNode(nodeRoot);

  // We do the same for the default Node
  secureServer.setDefaultNode(node404);
  insecureServer.setDefaultNode(node404);

  Serial.println("Starting HTTPS server...");
  secureServer.start();
  Serial.println("Starting HTTP server...");
  insecureServer.start();
  if (secureServer.isRunning() && insecureServer.isRunning()) {
    Serial.println("Servers ready.");
  }
}

void loop() {
  // We need to call both loop functions here
  secureServer.loop();
  insecureServer.loop();

  // Other code would go here...
  delay(1);
}

// The hanlder functions are the same as in the Static-Page example.
// The only difference is the check for isSecure in the root handler

void handleRoot(HTTPRequest * req, HTTPResponse * res) {
  res->setHeader("Content-Type", "text/html");

  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Hello World!</title></head>");
  res->println("<body>");
  res->println("<h1>Hello World!</h1>");

  res->print("<p>Your server is running for ");
  res->print((int)(millis()/1000), DEC);
  res->println(" seconds.</p>");

  // You can check if you are connected over a secure connection, eg. if you
  // want to use authentication and redirect the user to a secure connection
  // for that
  if (req->isSecure()) {
    res->println("<p>You are connected via <strong>HTTPS</strong>.</p>");
  } else {
    res->println("<p>You are connected via <strong>HTTP</strong>.</p>");
  }

  res->println("</body>");
  res->println("</html>");
}

void handle404(HTTPRequest * req, HTTPResponse * res) {
  req->discardRequestBody();
  res->setStatusCode(404);
  res->setStatusText("Not Found");
  res->setHeader("Content-Type", "text/html");
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Not Found</title></head>");
  res->println("<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>");
  res->println("</html>");
}
