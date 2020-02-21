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
 *  - Run a middleware that logs every request
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

// For the middleware
#include <functional>

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
void handleRoot(HTTPRequest * req, HTTPResponse * res);
void handle404(HTTPRequest * req, HTTPResponse * res);

// Declare a middleware function.
// Parameters:
// req: Request data, can be used to access URL, HTTP Method, Headers, ...
// res: Response data, can be used to access HTTP Status, Headers, ...
// next: This function is used to pass control down the chain. If you have done your work
//       with the request object, you may decide if you want to process the request.
//       If you do so, you call the next() function, and the next middleware function (if
//       there is any) or the actual requestHandler will be called.
//       If you want to skip the request, you do not call next, and set for example status
//       code 403 on the response to show that the user is not allowed to access a specific
//       resource.
//       The Authentication examples provides more details on this.
void middlewareLogging(HTTPRequest * req, HTTPResponse * res, std::function<void()> next);

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
  ResourceNode * nodeRoot    = new ResourceNode("/", "GET", &handleRoot);
  ResourceNode * node404     = new ResourceNode("", "GET", &handle404);

  // Add the root node to the server
  secureServer.registerNode(nodeRoot);

  // Add the 404 not found node to the server.
  // The path is ignored for the default node.
  secureServer.setDefaultNode(node404);

  // Add the middleware. The function will be called globally for every request
  // Note: The functions are called in the order they are added to the server.
  // Also, if you want a middleware to handle only specific requests, you can check
  // the URL within the middleware function.
  secureServer.addMiddleware(&middlewareLogging);

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

// We want to log the following information for every request:
// - Response Status
// - Request Method
// - Request String (URL + Parameters)
void middlewareLogging(HTTPRequest * req, HTTPResponse * res, std::function<void()> next) {
  // We want to print the response status, so we need to call next() first.
  next();
  // After the call, the status is (hopefully) set by the handler function, so we can
  // access it for logging.
  Serial.printf("middlewareLogging(): %3d\t%s\t\t%s\n",
      // Status code (like: 200)
      res->getStatusCode(),
      // Method used for the request (like: GET)
      req->getMethod().c_str(),
      // Request string (like /index.html)
      req->getRequestString().c_str());
}

// For details on the implementation of the hanlder functions, refer to the Static-Page example.
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
