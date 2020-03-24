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

// We will use SPIFFS and FS
#include <SPIFFS.h>
#include <FS.h>

// Includes for the server
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <HTTPBodyParser.hpp>
#include <HTTPMultipartBodyParser.hpp>
#include <HTTPURLEncodedBodyParser.hpp>

// We need to specify some content-type mapping, so the resources get delivered with the
// right content type and are displayed correctly in the browser
char contentTypes[][2][32] = {
  {".txt", "text/plain"},
  {".png",  "image/png"},
  {".jpg",  "image/jpg"},
  {"", ""}
};

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
void handleFormUpload(HTTPRequest * req, HTTPResponse * res);
void handleFormEdit(HTTPRequest * req, HTTPResponse * res);
void handleFile(HTTPRequest * req, HTTPResponse * res);
void handleDirectory(HTTPRequest * req, HTTPResponse * res);
void handle404(HTTPRequest * req, HTTPResponse * res);

std::string htmlEncode(std::string data) {
  // Quick and dirty: doesn't handle control chars and such.
  const char *p = data.c_str();
  std::string rv = "";
  while(p && *p) {
    char escapeChar = *p++;
    switch(escapeChar) {
      case '&': rv += "&amp;"; break;
      case '<': rv += "&lt;"; break;
      case '>': rv += "&gt;"; break;
      case '"': rv += "&quot;"; break;
      case '\'': rv += "&#x27;"; break;
      case '/': rv += "&#x2F;"; break;
      default: rv += escapeChar; break;
    }
  }
  return rv;
}

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

  // Setup filesystem
  if (!SPIFFS.begin(true)) Serial.println("Mounting SPIFFS failed");

  // For every resource available on the server, we need to create a ResourceNode
  // The ResourceNode links URL and HTTP method to a handler function
  ResourceNode * nodeRoot = new ResourceNode("/", "GET", &handleRoot);
  ResourceNode * nodeFormUpload = new ResourceNode("/upload", "POST", &handleFormUpload);
  ResourceNode * nodeFormEdit = new ResourceNode("/edit", "GET", &handleFormEdit);
  ResourceNode * nodeFormEditDone = new ResourceNode("/edit", "POST", &handleFormEdit);
  ResourceNode * nodeDirectory = new ResourceNode("/public", "GET", &handleDirectory);
  ResourceNode * nodeFile = new ResourceNode("/public/*", "GET", &handleFile);

  // 404 node has no URL as it is used for all requests that don't match anything else
  ResourceNode * node404  = new ResourceNode("", "GET", &handle404);

  // Add the root nodes to the server
  secureServer.registerNode(nodeRoot);
  secureServer.registerNode(nodeFormUpload);
  secureServer.registerNode(nodeFormEdit);
  secureServer.registerNode(nodeFormEditDone);
  secureServer.registerNode(nodeDirectory);
  secureServer.registerNode(nodeFile);

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
  res->println("<head><title>Very simple file server</title></head>");
  res->println("<body>");
  res->println("<h1>Very simple file server</h1>");
  res->println("<p>This is a very simple file server to demonstrate the use of POST forms. </p>");
  res->println("<h2>List existing files</h2>");
  res->println("<p>See <a href=\"/public\">/public</a> to list existing files and retrieve or edit them.</p>");
  res->println("<h2>Upload new file</h2>");
  res->println("<p>This form allows you to upload files (text, jpg and png supported best). It demonstrates multipart/form-data.</p>");
  res->println("<form method=\"POST\" action=\"/upload\" enctype=\"multipart/form-data\">");
  res->println("file: <input type=\"file\" name=\"file\"><br>");
  res->println("<input type=\"submit\" value=\"Upload\">");
  res->println("</form>");
  res->println("</body>");
  res->println("</html>");
}

void handleFormUpload(HTTPRequest * req, HTTPResponse * res) {
  // First, we need to check the encoding of the form that we have received.
  // The browser will set the Content-Type request header, so we can use it for that purpose.
  // Then we select the body parser based on the encoding.
  // Actually we do this only for documentary purposes, we know the form is going
  // to be multipart/form-data.
  HTTPBodyParser *parser;
  std::string contentType = req->getHeader("Content-Type");
  size_t semicolonPos = contentType.find(";");
  if (semicolonPos != std::string::npos) {
    contentType = contentType.substr(0, semicolonPos);
  }
  if (contentType == "multipart/form-data") {
    parser = new HTTPMultipartBodyParser(req);
  } else {
    Serial.printf("Unknown POST Content-Type: %s\n", contentType.c_str());
    return;
  }
  // We iterate over the fields. Any field with a filename is uploaded
  res->println("<html><head><title>File Upload</title></head><body><h1>File Upload</h1>");
  bool didwrite = false;
  while(parser->nextField()) {
    std::string name = parser->getFieldName();
    std::string filename = parser->getFieldFilename();
    std::string mimeType = parser->getFieldMimeType();
    Serial.printf("handleFormUpload: field name='%s', filename='%s', mimetype='%s'\n", name.c_str(), filename.c_str(), mimeType.c_str());
    // Double check that it is what we expect
    if (name != "file") {
      Serial.println("Skipping unexpected field");
      break;
    }
    // Should check file name validity and all that, but we skip that.
    std::string pathname = "/public/" + filename;
    File file = SPIFFS.open(pathname.c_str(), "w");
    size_t fileLength = 0;
    didwrite = true;
    while (!parser->endOfField()) {
      byte buf[512];
      size_t readLength = parser->read(buf, 512);
      file.write(buf, readLength);
      fileLength += readLength;
    }
    file.close();
    res->printf("<p>Saved %d bytes to %s</p>", (int)fileLength, pathname.c_str());
  }
  if (!didwrite) {
    res->println("<p>Did not write any file</p>");
  }
  res->println("</body></html>");
  delete parser;
}

void handleFormEdit(HTTPRequest * req, HTTPResponse * res) {
  if (req->getMethod() == "GET") {
    // Initial request. Get filename from request parameters and return form.
    auto params = req->getParams();
    std::string filename;
    bool hasFilename = params->getQueryParameter("filename", filename);
    std::string pathname = std::string("/public/") + filename;
    res->println("<html><head><title>Edit File</title><head><body>");
    File file = SPIFFS.open(pathname.c_str());
    if (!hasFilename) {
      res->println("<p>No filename specified.</p>");
    } else if (!file.available()) {
      res->printf("<p>File not found: %s</p>\n", pathname.c_str());
    } else {
      res->printf("<h2>Edit content of %s</h2>\n", pathname.c_str());
      res->println("<form method=\"POST\" enctype=\"application/x-www-form-urlencoded\">");
      res->printf("<input name=\"filename\" type=\"hidden\" value=\"%s\">", filename.c_str());
      res->print("<textarea name=\"content\" rows=\"24\" cols=\"80\">");
      // Read the file and write it to the response
      size_t length = 0;
      do {
        char buffer[256];
        length = file.read((uint8_t *)buffer, 256);
        std::string bufferString(buffer, length);
        bufferString = htmlEncode(bufferString);
        res->write((uint8_t *)bufferString.c_str(), bufferString.size());
      } while (length > 0);
      res->println("</textarea><br>");
      res->println("<input type=\"submit\" value=\"Save\">");
      res->println("</form>");
    }
    res->println("</body></html>");
  } else { // method != GET
    // Assume POST request. Contains submitted data.
    res->println("<html><head><title>File Edited</title><head><body><h1>File Edited</h1>");
    HTTPURLEncodedBodyParser parser(req);
    std::string filename;
    bool savedFile = false;
    while(parser.nextField()) {
      std::string name = parser.getFieldName();
      if (name == "filename") {
        char buf[512];
        size_t readLength = parser.read((byte *)buf, 512);
        filename = std::string("/public/") + std::string(buf, readLength);
      } else if (name == "content") {
        if (filename == "") {
          res->println("<p>Error: form contained content before filename.</p>");
          break;
        }
        size_t fieldLength = 0;
        File file = SPIFFS.open(filename.c_str(), "w");
        savedFile = true;
        while (!parser.endOfField()) {
          byte buf[512];
          size_t readLength = parser.read(buf, 512);
          file.write(buf, readLength);
          fieldLength += readLength;
        }
        file.close();
        res->printf("<p>Saved %d bytes to %s</p>", int(fieldLength), filename.c_str());
      } else {
        res->printf("<p>Unexpected field %s</p>", name.c_str());
      }
    }
    if (!savedFile) {
      res->println("<p>No file to save...</p>");
    }
    res->println("</body></html>");
  }
}

void handleDirectory(HTTPRequest * req, HTTPResponse * res) {
  res->println("<html><head><title>File Listing</title><head><body>");
  File d = SPIFFS.open("/public");
  if (!d.isDirectory()) {
    res->println("<p>No files found.</p>");
  } else {
    res->println("<h1>File Listing</h1>");
    res->println("<ul>");
    File f = d.openNextFile();
    while (f) {
      std::string pathname(f.name());
      res->printf("<li><a href=\"%s\">%s</a>", pathname.c_str(), pathname.c_str());
      if (pathname.rfind(".txt") != std::string::npos) {
        std::string filename = pathname.substr(8); // Remove /public/
        res->printf(" <a href=\"/edit?filename=%s\">[edit]</a>", filename.c_str());
      }
      res->println("</li>");
      f = d.openNextFile();
    }
    res->println("</ul>");
  }
  res->println("</body></html>");
}

void handleFile(HTTPRequest * req, HTTPResponse * res) {
  std::string filename = req->getRequestString();
  // Check if the file exists
  if (!SPIFFS.exists(filename.c_str())) {
    // Send "404 Not Found" as response, as the file doesn't seem to exist
    res->setStatusCode(404);
    res->setStatusText("Not found");
    res->println("404 Not Found");
    return;
  }

  File file = SPIFFS.open(filename.c_str());

  // Set length
  res->setHeader("Content-Length", httpsserver::intToString(file.size()));

  // Content-Type is guessed using the definition of the contentTypes-table defined above
  int cTypeIdx = 0;
  do {
    if(filename.rfind(contentTypes[cTypeIdx][0])!=std::string::npos) {
      res->setHeader("Content-Type", contentTypes[cTypeIdx][1]);
      break;
    }
    cTypeIdx+=1;
  } while(strlen(contentTypes[cTypeIdx][0])>0);

  // Read the file and write it to the response
  uint8_t buffer[256];
  size_t length = 0;
  do {
    length = file.read(buffer, 256);
    res->write(buffer, length);
  } while (length > 0);

  file.close();
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
