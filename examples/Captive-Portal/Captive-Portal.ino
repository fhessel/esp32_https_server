/**
 * Example for the ESP32 HTTP(S) Webserver
 *
 * This script will create a captive portal. A captive portal is an access point
 * that resolves all DNS requests to a specific IP address (in this case its own)
 * where it hosts a webserver. Then, it redirects the user to a well-known hostname
 * from where it serves a website.
 * 
 * Usually this is used for providing a login page in a public WiFi network. For
 * that specific use case, it exists an API:
 * https://tools.ietf.org/html/draft-ietf-capport-api-08
 * However, this approach needs an upstream internet connection and valid certificated.
 * 
 * Another option is to use DHCP option 114 to provide the URL of a captive portal, but
 * configuring custom option types for the DHCP server is a bit tricky in Arduino.
 * 
 * So this is really the basic example: We will redirect users to a website when they're
 * connected to the access point. If the client has other means of connecting to
 * the Internet available, this might not work, as external DNS servers might be in use.
 * This server will not redirect the client to the portal.
 * 
 * Please also note that the Arduino DNS server is quite hacky. It can only process very
 * specific requests, so it might not work for every client (in particular: if you like
 * to use "dig" for debugging, don't.)
 */

// C O N F I G U R A T I O N - - - - - - - - - - - - - - - - - -
// The hostname to redirect to.
// You can use either a hostname (arbitrary, like "captive.esp") or an IP address. Using
// the IP address is preferable, as this circumvents the issue that the client resolves
// the local hostname on an external DNS and will not be guided to the captive portal.
// Must start with http://
const char *hosturl = "http://192.168.8.1";

// The name of the access point
const char *apname = "CaptiveESP";

// Subnet configuration. When using an IP as hostname, make sure it is the same as awip.
IPAddress apip(192, 168, 8, 1);
IPAddress gwip(192, 168, 8, 1);
IPAddress apnetmask(255, 255, 255, 0);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// We will use wifi
#include <WiFi.h>

// We need to run a DNS server
#include <DNSServer.h>

// Required for the middleware
#include <functional>

// We include the server. For the captive portal, we will use the HTTP server.
#include <HTTPServer.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

// The server comes in a separate namespace. For easier use, include it here.
using namespace httpsserver;

// We instantiate the web server with the default parameters
HTTPServer portalServer = HTTPServer();

// Same for the DNS server
DNSServer dnsServer = DNSServer();

// We only define a single page to show the general operation of the captive portal.
// For 404 etc, we use the webserver default. You can have a look at the others
// examples to make your project fancier.
void handleRoot(HTTPRequest * req, HTTPResponse * res);

// This middleware intercepts every request. If it notices that the request is not
// for the target host (configured above), it will send a redirect.
void captiveMiddleware(HTTPRequest * req, HTTPResponse * res, std::function<void()> next);

void setup() {
  // For logging
  Serial.begin(115200);

  // 1) Configure the access point
  // Depending on what you did before with your ESP, you might face the problem of
  // getting GURU medidations every time some client connects to the access point.
  // At the end of this sketch, you'll find instructions for a workaround.
  Serial.print("Setting up WiFi... ");
  // Disable STA mode, if still active
  WiFi.disconnect();
  // Do not uses connection config persistence
  WiFi.persistent(false);
  // Start the SoftAP
  WiFi.softAP(apname);
  // Reconfigure the AP IP. Wait until it's done.
  while (!(WiFi.softAPIP() == apip)) {
    WiFi.softAPConfig(apip, gwip, apnetmask);
  }
  Serial.println("OK");

  // 2) Configure DNS
  // All hostnames are belong to us (we let a wildcard point to our AP)
  Serial.print("Starting DNS... ");
  if (!dnsServer.start(53, "*", apip)) {
    Serial.println("failed");
    while(true);
  }
  Serial.println("OK");

  // 3) Configure the server
  // We create the single node and store it on the server.
  portalServer.registerNode(new ResourceNode("/", "GET", &handleRoot));
  // We also register our middleware
  portalServer.addMiddleware(&captiveMiddleware);
  // Then we start the server
  Serial.print("Starting HTTP server... ");
  portalServer.start();
  if (portalServer.isRunning()) {
    Serial.println("OK");
  } else {
    Serial.println("failed");
    while(true);
  }
}

void loop() {
  // In the main loop, we now need to process both, DNS and HTTP
  portalServer.loop();
  dnsServer.processNextRequest();
}

// This function intercepts each request. See the middleware-examples for more details.
// The goal is to identify whether the client has already been redirected to the configured
// hostname, and if not, to trigger this redirect. Being redirected on arbitrary domains
// is a way how some operating systems detect the presence of a captive portal.
void captiveMiddleware(HTTPRequest * req, HTTPResponse * res, std::function<void()> next) {
  // To check if we have already redirected, we need the "host" HTTP header
  HTTPHeaders *headers = req->getHTTPHeaders();
  std::string hdrHostname = headers->getValue("host");
  // If the hostname is not what we redirect to...
  if (hdrHostname != &hosturl[7]) { // cutoff the http://
    // ... we start a temporary redirect ...
    res->setStatusCode(302);
    res->setStatusText("Found");
    // ... to this hostname ...
    res->setHeader("Location", hosturl);
    // ... and stop processing the request.
    return;
  }
  // Otherwise, the request will be forwarded (and most likely reach the handleRoot function)
  next();
}

// Main page of the captive portal
void handleRoot(HTTPRequest * req, HTTPResponse * res) {
  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");
  res->println(
    "<!DOCTYPE html>"
    "<html>"
    "<head><title>Captive Portal</title></head>"
    "<body>"
    "<h1>Captive Portal</h1>"
    "<p>Ha, gotcha!</p>"
    "</body>"
    "</html>"
  );
}

// Workaround for GURU meditation on new connections to the ESP32
// The most likely reason is some broken WiFi configuration in the flash of your ESP32
// that does not go away, even with WiFi.persistent(false) or re-flashing the sketch.
// This broken configuration resides in the nvm partition of the ESP.
// 
// Partitions on the ESP are a bit different from what you know from your computer. They
// have a specific type and can contain either data, configuration or an application image.
// The WiFi configuration is placed in the "nvm" partition.
//
// If you know how to use esptool, read the partition table from 0x8000..0x9000, look for
// the nvm partition and clear it using esptool erase_region <offset> <length>
//
// If you are not familiar with the esptool, you can erase the whole flash.
// Make sure only one board is connected to your computer, then run:
//  esptool.py erase_flash
// After that, flash your sketch again.
// You get esptool from https://github.com/espressif/esptool
