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
 *  - Show page on web server root that will display some SVG smileys.
 *    Using a simple HTML form and GET parameters, you can change the colors
 *    of those images.
 *  - Provide the svg image with an optional GET parameter that allows to
 *    modify the background color, URL: /images/awesome.svg
 *  - Provide an example that shows how wildcard URL parameters get parsed
 *    URL: /urlparam/some/thing
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

// We declare some handler functions (definition at the end of the file)
void handleRoot(HTTPRequest * req, HTTPResponse * res);
void handleSVG(HTTPRequest * req, HTTPResponse * res);
void handleQueryDemo(HTTPRequest * req, HTTPResponse * res);
void handlePathParam(HTTPRequest * req, HTTPResponse * res);
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
  ResourceNode * nodeRoot      = new ResourceNode("/", "GET", &handleRoot);
  ResourceNode * nodeSVG       = new ResourceNode("/images/awesome.svg", "GET", &handleSVG);
  ResourceNode * nodeQueryDemo = new ResourceNode("/queryparams", "GET", &handleQueryDemo);
  ResourceNode * node404       = new ResourceNode("", "GET", &handle404);

  // Path parameters
  // If you want (for example) to return a specific instance of an object type by its ID
  // you can use URLs like /led/1, led/2, ... - And you do not need to register one Resource
  // Node per ID, but you can use wildcards in the route definition. The following route
  // has two wildcards, and will match for example to /urlparam/foo/bar, where "foo" and "bar"
  // are accessible parameters in the handler function.
  // Note: The wildcards can only be used between slashes at the moment (so /urlparam* would
  // not work).
  ResourceNode * nodeURLParam = new ResourceNode("/urlparam/*/*", "GET", &handlePathParam);

  // Add the root node to the server
  secureServer.registerNode(nodeRoot);

  // Add the SVG image
  secureServer.registerNode(nodeSVG);

  // Query parameter demo
  secureServer.registerNode(nodeQueryDemo);

  // Add the path parameter
  // Note: The order of nodes may become important here. If you have one node for "/led" (e.g. list of LEDs)
  // and one node for /led/* (LED details), you should register the non-parameterized version first. The server
  // follows a first-match policy. If you would register the details node first, a call to /led/ will be targetted
  // at the details handler function with an empty parameter, which is probably not what you want.
  secureServer.registerNode(nodeURLParam);

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
  // We will deliver an HTML page
  res->setHeader("Content-Type", "text/html");

  // Write the response page
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Hello World!</title></head>");
  res->println("<style>.info{font-style:italic}</style>");
  res->println("<body>");

  res->println("<h1>Query Parameters</h1>");
  res->println("<p class=\"info\">The parameters after the question mark in your URL.</p>");

  // Show a form to select a color to colorize the faces
  // We pass the selection as get parameter "shades" to this very same page,
  // so we can evaluate it below
  res->println("<form method=\"GET\" action=\"/\">Show me faces in shades of ");
  res->println("<select name=\"shades\">");
  res->println("<option value=\"red\">red</option>");
  res->println("<option value=\"green\">green</option>");
  res->println("<option value=\"blue\">blue</option>");
  res->println("<option value=\"yellow\">yellow</option>");
  res->println("<option value=\"cyan\">cyan</option>");
  res->println("<option value=\"magenta\">magenta</option>");
  res->println("<option value=\"rainbow\">rainbow</option>");
  res->println("</select>");
  res->println("<button type=\"submit\">Go!</button>");
  res->println("</form>");

  // Get the params to check if the user did select something
  ResourceParameters * params = req->getParams();
  std::string paramName = "shades";

  // Print 6 faces
  for(int i = 0; i < 6; i++) {
    // Include the image of the handleSVG function with a specific color code
    res->print("<img style=\"height:100px;width:100px\" src=\"images/awesome.svg?color=");

    // Depending on the selection we show the images in a specific color shade
    // Default is dark gray.
    int r = 63, g = 63, b = 63;
    std::string paramVal;
    if (params->getQueryParameter(paramName, paramVal)) {
      if (paramVal == "red" || paramVal == "magenta" || paramVal == "yellow" || paramVal == "rainbow") {
        r = 128 + random(0, 128);
      }
      if (paramVal == "green" || paramVal == "cyan" || paramVal == "yellow" || paramVal == "rainbow") {
        g = 128 + random(0, 128);
      }
      if (paramVal == "blue" || paramVal == "magenta" || paramVal == "cyan" || paramVal == "rainbow") {
        b = 128 + random(0, 128);
      }
		}

    // Print the random color. As the HTTPResponse extends the Print interface, we can make use of that.
    res->print(r, HEX);
    res->print(g, HEX);
    res->print(b, HEX);

    res->print("\" alt=\"Awesome!\" />");
  }
  res->println("<p>You'll find another demo <a href=\"/queryparams?a=42&b&c=13&a=hello\">here</a>.</p>");

  // Link to the path parameter demo
  res->println("<h1>Path Parameters</h1>");
  res->println("<p class=\"info\">The parameters derived from placeholders in your path, like /foo/bar.</p>");
  res->println("<p>You'll find the demo <a href=\"/urlparam/foo/bar\">here</a>.</p>");

  res->println("</body>");
  res->println("</html>");
}

// This callback responds with an SVG image to a GET request. The icon is the "awesome face".
// (borrowed from https://commons.wikimedia.org/wiki/File:718smiley.svg)
//
// If the color query parameter is set (so the URL is like awesome.svg?color=fede58), the
// background of our awesome face is changed.
void handleSVG(HTTPRequest * req, HTTPResponse * res) {
  // Get access to the parameters
  ResourceParameters * params = req->getParams();

  // Set SVG content type
  res->setHeader("Content-Type", "image/svg+xml");

  // Set a default color
  std::string fillColor = "fede58";

  // Get request parameter (like awesome.svg?color=ff0000) and validate it
  std::string colorParamName = "color";

  // Check that the parameter is set and retrieve it.
  // The getQueryParameter function will modify the second parameter, but only if the query
  // parameter is set.
  std::string requestColor;
  if (params->getQueryParameter(colorParamName, requestColor)) {
    // Check for correct length
    if (requestColor.length()==6) {
      bool colorOk = true;
      // Check that we only have characters within [0-9a-fA-F]
      for(int i = 1; i < 6 && colorOk; i++) {
        if (!(
          (requestColor[i]>='0' && requestColor[i]<='9' ) ||
          (requestColor[i]>='a' && requestColor[i]<='f' ) ||
          (requestColor[i]>='A' && requestColor[i]<='F' )
        )) {
          colorOk = false;
        }
      }
      // If validation was successful, replace the default color
      if (colorOk) {
        fillColor = requestColor;
      }
    }
  }

  // Print the SVG to the response:
  res->print("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>");
  res->print("<svg id=\"svg1923\" width=\"733\" xmlns=\"http://www.w3.org/2000/svg\" height=\"733\">");
  res->print("<circle cy=\"366.5\" cx=\"366.5\" r=\"366.5\"/>");
  res->print("<circle cy=\"366.5\" cx=\"366.5\" r=\"336.5\" fill=\"#");
  // We insert the color from the parameter here
  res->printStd(fillColor);
  res->print("\"/>");
  res->print("<path d=\"m325 665c-121-21-194-115-212-233v-8l-25-1-1-18h481c6 13 10 27 13 41 13 94-38 146-114 193-45 23-93 29-142 26z\"/>");
  res->print("<path d=\"m372 647c52-6 98-28 138-62 28-25 46-56 51-87 4-20 1-57-5-70l-423-1c-2 56 39 118 74 157 31 34 72 54 116 63 11 2 38 2 49 0z\" fill=\"#871945\"/>");
  res->print("<path d=\"m76 342c-13-26-13-57-9-85 6-27 18-52 35-68 21-20 50-23 77-18 15 4 28 12 39 23 18 17 30 40 36 67 4 20 4 41 0 60l-6 21z\"/>");
  res->print("<path d=\"m234 323c5-6 6-40 2-58-3-16-4-16-10-10-14 14-38 14-52 0-15-18-12-41 6-55 3-3 5-5 5-6-1-4-22-8-34-7-42 4-57.6 40-66.2 77-3 17-1 53 4 59h145.2z\" fill=\"#fff\"/>");
  res->print("<path d=\"m378 343c-2-3-6-20-7-29-5-28-1-57 11-83 15-30 41-52 72-60 29-7 57 0 82 15 26 17 45 49 50 82 2 12 2 33 0 45-1 10-5 26-8 30z\"/>");
  res->print("<path d=\"m565 324c4-5 5-34 4-50-2-14-6-24-8-24-1 0-3 2-6 5-17 17-47 13-58-9-7-16-4-31 8-43 4-4 7-8 7-9 0 0-4-2-8-3-51-17-105 20-115 80-3 15 0 43 3 53z\" fill=\"#fff\"/>");
  res->print("<path d=\"m504 590s-46 40-105 53c-66 15-114-7-114-7s14-76 93-95c76-18 126 49 126 49z\" fill=\"#f9bedd\"/>");
  res->print("</svg>");
}

// This is a more generic demo for the query parameters. It makes use of the iterator
// interface to access them, which is useful if you do not know the paramter names in
// adavance.
void handleQueryDemo(HTTPRequest * req, HTTPResponse * res) {
  // A word of warning: In this example, we use the query parameters and directly print
  // them into the HTML output. We do this to simplify the demo. NEVER do this in a
  // real application, as it allows cross-site-scripting.
  res->setHeader("Content-Type", "text/html");

  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head>");
  res->println("<title>Query Parameter Demo</title>");
  res->println("</head>");
  res->println("<body>");
  res->println("<p>The following query paramters have been set:</p>");
  
  // Start a table to display the parameters
  res->println("<table style=\"border:1px solid black collapse;\">");
  res->println("<tr><th>Key</th><th>Value</th></tr>");
  // Iterate over the parameters. For more information, read about the C++ standard template library, 
  // especially about vectors and iterators.
  ResourceParameters *params = req->getParams();
  for(auto it = params->beginQueryParameters(); it != params->endQueryParameters(); ++it) {
    res->print("<tr><td>");
    
    // The iterator yields std::pairs of std::strings. The first value contains the parameter key
    res->printStd((*it).first);
    res->print("</td><td>");

    // and the second value contains the parameter value
    res->printStd((*it).second);
    res->println("</td></tr>");
  }
  res->println("</table>");

  // You can retrieve the total parameter count from the parameters instance:
  res->print("<p>There are a total of ");
  res->print(params->getQueryParameterCount());
  res->print(" parameters, with ");
  res->print(params->getQueryParameterCount(true));
  res->println(" unique keys.</p>");

  res->println("<p>Go <a href=\"/\">back to main page</a>.</p>");
  res->println("</body>");
  res->println("</html>");
}

// This is a simple handler function that will show the content of URL parameters.
// If you call for example /urlparam/foo/bar, you will get the parameter values
// "foo" and "bar" provided by the ResourceParameters.
void handlePathParam(HTTPRequest * req, HTTPResponse * res) {
  // Get access to the parameters
  ResourceParameters * params = req->getParams();

  // Set a simple content type
  res->setHeader("Content-Type", "text/plain");

  // The url pattern is: /urlparam/*/*
  // This will make the content for the first parameter available on index 0,
  // and the second wildcard as index 1.
  // getPathParameter will - like getQueryParameter - write the value to the second parameter,
  // and return true, if the index is valid. Otherwise it returns false and leaves the second
  // parameter as it is.

  std::string parameter1, parameter2;
  // Print the first parameter value
  if (params->getPathParameter(0, parameter1)) {
    res->print("Parameter 1: ");
    res->printStd(parameter1);
  }

  res->println();

  // Print the second parameter value
  if (params->getPathParameter(1, parameter2)) {
    res->print("Parameter 2: ");
    res->printStd(parameter2);
  }

  res->println("\n\nChange the parameters in the URL to see how they get parsed!");
}

// For details to this function, see the Static-Page example
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
