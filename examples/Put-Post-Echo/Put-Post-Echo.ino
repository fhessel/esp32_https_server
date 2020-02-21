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
 *  - Provide an Echo-Service for PUT/POST requests on /
 *  - 404 for everything else
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
// The contstructor takes some more parameters, but we go for default values here.
HTTPSServer secureServer = HTTPSServer(&cert);

// Declare some handler functions for the various URLs on the server
// The signature is always the same for those functions. They get two parameters,
// which are pointers to the request data (read request body, headers, ...) and
// to the response data (write response, set status code, ...)
void handleRoot(HTTPRequest * req, HTTPResponse * res);
void handleEcho(HTTPRequest * req, HTTPResponse * res);
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

  // Register the echo handler. You can use the same handler function for multiple
  // nodes. Note also that we now have three resource nodes for the / URL, so
  // the server uses only the method to distinguish between them.
  ResourceNode * nodeEchoPut  = new ResourceNode("/", "PUT", &handleEcho);
  ResourceNode * nodeEchoPost = new ResourceNode("/", "POST", &handleEcho);

  // Add the root node to the server
  secureServer.registerNode(nodeRoot);

  // Add the nodes for the echo service
  secureServer.registerNode(nodeEchoPut);
  secureServer.registerNode(nodeEchoPost);

  // Add the 404 not found node to the server.
  // The path is ignored for the default node.
  secureServer.setDefaultNode(node404);

  Serial.println("Starting server...");
  secureServer.start();
  if (secureServer.isRunning()) {
    Serial.println("Server ready.");
  }
}

void loop() {
  // This call will let the server do its work
  secureServer.loop();

  // Other code would go here...
  delay(1);
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
  res->print("<p>The easiest way to test the echo function is to use an HTTP debugging "
      " tool where you are able to set a body and then let it call PUT or POST on"
      " https://");
  res->print(WiFi.localIP());
  res->print("/. Otherwise you can use command line tools like curl:</p>"
      "<pre>cat &lt;&lt;EOF | curl --insecure -X PUT -d @- https://");
  res->print(WiFi.localIP());
  res->print("/\nHere goes your request \nbody\n"
      "EOF</pre>");
  res->println("</body>");
  res->println("</html>");
}

void handleEcho(HTTPRequest * req, HTTPResponse * res) {
  // The echo callback will return the request body as response body.

  // We use text/plain for the response
  res->setHeader("Content-Type","text/plain");

  // Stream the incoming request body to the response body
  // Theoretically, this should work for every request size.
  byte buffer[256];
  // HTTPReqeust::requestComplete can be used to check whether the
  // body has been parsed completely.
  while(!(req->requestComplete())) {
    // HTTPRequest::readBytes provides access to the request body.
    // It requires a buffer, the max buffer length and it will return
    // the amount of bytes that have been written to the buffer.
    size_t s = req->readBytes(buffer, 256);

    // The response does not only implement the Print interface to
    // write character data to the response but also the write function
    // to write binary data to the response.
    res->write(buffer, s);
  }
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
