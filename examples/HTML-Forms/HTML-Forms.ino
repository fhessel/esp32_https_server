/**
 * Example for the ESP32 HTTP(S) Webserver
 *
 * IMPORTANT NOTE:
 * To run this script, you need to
 *  1) Enter your WiFi SSID and PSK below this comment
 *  2) Make sure to have certificate data available. You will find a
 *     shell script and instructions to do so in the library folder
 *     under extras/
 *
 * This script will install an HTTPS Server on your ESP32 with the following
 * functionalities:
 *  - Show simple page on web server root that includes some HTML Forms
 *  - Define a POST handler that handles the forms using the HTTPBodyParser API
 *    provided by the library.
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
void handleForm(HTTPRequest * req, HTTPResponse * res);
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
  ResourceNode * nodeForm = new ResourceNode("/", "POST", &handleForm);

	// 404 node has no URL as it is used for all requests that don't match anything else
  ResourceNode * node404  = new ResourceNode("", "GET", &handle404);

  // Add the root nodes to the server
  secureServer.registerNode(nodeRoot);
  secureServer.registerNode(nodeForm);

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
	res->println("<h1>HTML Forms</h1>");
	res->println("<p>This page contains some forms to show you how form data can be evaluated on server side.");

	// The following forms will all use the same target (/ - the server's root directory) and POST method, so
	// the data will go to the request body. They differ on the value of the enctype though.

	// enctype=x-www-form-urlencoded
	// means that the parameters of form elements will be encoded like they would
	// be encoded if you would use GET as method and append them to the URL (just after a ? at the end of the
	// resource path). Different fields are separated by an &-character. Special characters have a specific encoding
	// using the %-character, so for example %20 is the representation of a space.
	// The body could look like this:
	//
	// foo=bar&bat=baz
	//
	// Where foo and bat are the variables and bar and baz the values.
	//
	// Advantages:
	// - Low overhead
	// Disadvantages:
	// - May be hard to read for humans
	// - Cannot be used for inputs with type=file (you will only get the filename, not the content)
	res->println("<form method=\"POST\" action=\"/\" enctype=\"x-www-form-urlencoded\">");
	res->println("<h2>enctype=x-www-form-urlencoded</h2>");
	res->println("</form>")

	// enctype=multipart/form-data
	//
	// TODO: Explanatory text
	//
	// Advantages:
	// - Even longer text stays somewhat human readable
	// - Can be used for files and binary data
	// Disadvantages:
	// - Big overhead if used for some small string values
	res->println("<form method=\"POST\" action=\"/\" enctype=\"multipart/form-data\">");
	res->println("<h2>enctype=multipart/form-data</h2>");
	res->println("</form>")

	res->println("</body>");
	res->println("</html>");
}

void handleForm(HTTPRequest * req, HTTPResponse * res) {
	// First, we need to check the encoding of the form that we have received.
	// The browser will set the Content-Type request header, so we can use it for that purpose.
	// Then we select the body parser based on the encoding.
	// Note: This is only necessary if you expect various enctypes to be send to the same handler.
	// If you would have only one form on your page with a fixed enctype, just instantiate the
	// corresponding reader.

	// TODO: Select Parser, instantiate it

	// Now we iterate over the so-called fields of the BodyParser. This works in the same way for
	// all body parsers.
	// The interface is somewhat limited, so you cannot just call something like
	// myParser.get("fieldname"), because this would require the parser to cache the whole request
	// body. If you have your ESP32 attached to an external SD Card and you want to be able to upload
	// files that are larger than the ESP's memory to that card, this would not work.
	// This is why you iterate over the fields by using myParser.next() and check the name of the
	// current field with myParser.getFieldName(), and then process it with a buffer.
	// If you need random access to all fields' values, you need to parse them into a map or some similar
	// data structure by yourself and make sure that all fits into the memory.

	// TODO: Iterate over fields
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

	// Write a tiny HTML page
	res->println("<!DOCTYPE html>");
	res->println("<html>");
	res->println("<head><title>Not Found</title></head>");
	res->println("<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>");
	res->println("</html>");
}
