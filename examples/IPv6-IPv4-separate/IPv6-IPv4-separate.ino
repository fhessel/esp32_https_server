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

// First, we create an IPv4 HTTPSServer with the certificate created above
HTTPSServer ipv4Server = HTTPSServer(&cert);

// Additionally, we create an HTTPSServer for IPv6
// We need to manually specify all the parameters
// But the important ones are the address to bind to
// And the flag to only accept IPv6 connections
// - This makes sure the two servers don't interfere
HTTPSServer ipv6Server = HTTPSServer(&cert, (uint16_t)443U, (uint8_t)'\004', in6addr_any.s6_addr, true);

// Declare some handler functions for the various URLs on the server
void handleRootIPv4(HTTPRequest * req, HTTPResponse * res);
void handleRootIPv6(HTTPRequest * req, HTTPResponse * res);
void handle404(HTTPRequest * req, HTTPResponse * res);

void WiFiEvent(WiFiEvent_t event);

void setup() {
  // For logging
  Serial.begin(115200);

  // Connect to WiFi
  Serial.println("Setting up WiFi");
  // Handle WiFi events, see WifiEvent()
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(WIFI_SSID, WIFI_PSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.print("Connected. IPv4=");
  Serial.println(WiFi.localIP());

  // For every resource available on the server, we need to create a ResourceNode
  // The ResourceNode links URL and HTTP method to a handler function
  ResourceNode * nodeRootIPv4 = new ResourceNode("/", "GET", &handleRootIPv4);
  ResourceNode * nodeRootIPv6 = new ResourceNode("/", "GET", &handleRootIPv6);
  ResourceNode * node404  = new ResourceNode("", "GET", &handle404);

  // Add the root node to the servers. We can use the same ResourceNode on multiple
  // servers (you could also run multiple HTTPS servers)
  ipv4Server.registerNode(nodeRootIPv4);
  ipv6Server.registerNode(nodeRootIPv6);

  // We do the same for the default Node
  ipv4Server.setDefaultNode(node404);
  ipv6Server.setDefaultNode(node404);

  Serial.println("Starting IPv4 server...");
  ipv4Server.start();
  Serial.println("Starting IPv6 server...");
  ipv6Server.start();
  if (ipv4Server.isRunning() && ipv6Server.isRunning()) {
    Serial.println("Servers ready.");
  }
}

void loop() {
  // We need to call both loop functions here
  ipv4Server.loop();
  ipv6Server.loop();

  // Other code would go here...
  delay(1);
}

// We serve different content on the IPv4 and IPv6 servers
// Therefore we create two different handlers

void handleRootIPv4(HTTPRequest * req, HTTPResponse * res) {
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

  res->println("<p>You are connected to the <strong>IPv4</strong> server.</p>");
  res->print("<p>Your IP address is ");
  res->print(req->getClientIP());
  res->println(".</p>");

  res->println("</body>");
  res->println("</html>");
}

void handleRootIPv6(HTTPRequest * req, HTTPResponse * res) {
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

  res->println("<p>You are connected to the <strong>IPv6</strong> server.</p>");
  res->print("<p>Your IP address is ");
  res->print(req->getClientIPv6());
  res->println(".</p>");

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

// We need to enable IPv6 on the wifi client AFTER we are connected
// When we get an IPv6 address, we print it
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
        case SYSTEM_EVENT_STA_CONNECTED:
            // Enable IPv6
            WiFi.enableIpV6();
            break;
        case SYSTEM_EVENT_AP_STA_GOT_IP6:
            Serial.print("Connected. IPv6=");
            Serial.println(WiFi.localIPv6());
            break;
        default:
            break;
    }
}
