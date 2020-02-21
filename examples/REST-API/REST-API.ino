/**
 * Example for the ESP32 HTTP(S) Webserver
 *
 * IMPORTANT NOTE:
 * This example is a bit more complex than the other ones, so be careful to 
 * follow all steps.
 * 
 * Make sure to check out the more basic examples like Static-Page to understand
 * the fundamental principles of the API before proceeding with this sketch.
 * 
 * To run this script, you need to
 *  1) Enter your WiFi SSID and PSK below this comment
 *  2) Install the SPIFFS File uploader into your Arduino IDE to be able to
 *     upload static data to the webserver.
 *     Follow the instructions at:
 *     https://github.com/me-no-dev/arduino-esp32fs-plugin
 *  3) Upload the static files from the data/ directory of the example to your
 *     module's SPIFFs by using "ESP32 Sketch Data Upload" from the tools menu.
 *     If you face any problems, read the description of the libraray mentioned
 *     above.
 *     Note: If mounting SPIFFS fails, the script will wait for a serial connection
 *     (open your serial monitor!) and ask if it should format the SPIFFS partition.
 *     You may need this before uploading the data
 *     Note: Make sure to select a partition layout that allows for SPIFFS in the
 *     boards menu
 *  4) Have the ArduinoJSON library installed and available. (Tested with Version 5.13.4)
 *     You'll find it at:
 *     https://arduinojson.org/
 *
 * This script will install an HTTPS Server on your ESP32 with the following
 * functionalities:
 *  - Serve static files from the SPIFFS's data/public directory
 *  - Provide a REST API at /api to receive the asynchronous http requests
 *    - /api/uptime provides access to the current system uptime
 *    - /api/events allows to register or delete events to turn PINs on/off
 *      at certain times.
 *  - Use Arduino JSON for body parsing and generation of responses.
 *  - The certificate is generated on first run and stored to the SPIFFS in
 *    the cert directory (so that the client cannot retrieve the private key)
 */

// TODO: Configure your WiFi here
#define WIFI_SSID "<your ssid goes here>"
#define WIFI_PSK  "<your pre-shared key goes here>"

// We will use wifi
#include <WiFi.h>

// We will use SPIFFS and FS
#include <SPIFFS.h>
#include <FS.h>

// We use JSON as data format. Make sure to have the lib available
#include <ArduinoJson.h>

// Working with c++ strings
#include <string>

// Define the name of the directory for public files in the SPIFFS parition
#define DIR_PUBLIC "/public"

// We need to specify some content-type mapping, so the resources get delivered with the
// right content type and are displayed correctly in the browser
char contentTypes[][2][32] = {
  {".html", "text/html"},
  {".css",  "text/css"},
  {".js",   "application/javascript"},
  {".json", "application/json"},
  {".png",  "image/png"},
  {".jpg",  "image/jpg"},
  {"", ""}
};

// Includes for the server
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <util.hpp>

// The HTTPS Server comes in a separate namespace. For easier use, include it here.
using namespace httpsserver;

SSLCert * getCertificate();
void handleSPIFFS(HTTPRequest * req, HTTPResponse * res);
void handleGetUptime(HTTPRequest * req, HTTPResponse * res);
void handleGetEvents(HTTPRequest * req, HTTPResponse * res);
void handlePostEvent(HTTPRequest * req, HTTPResponse * res);
void handleDeleteEvent(HTTPRequest * req, HTTPResponse * res);

// We use the following struct to store GPIO events:
#define MAX_EVENTS 20
struct {
  // is this event used (events that have been run will be set to false)
  bool active;
  // when should it be run?
  unsigned long time;
  // which GPIO should be changed?
  int gpio;
  // and to which state?
  int state;
} events[MAX_EVENTS];

// We just create a reference to the server here. We cannot call the constructor unless
// we have initialized the SPIFFS and read or created the certificate
HTTPSServer * secureServer;

void setup() {
  // For logging
  Serial.begin(115200);

  // Set the pins that we will use as output pins
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(33, OUTPUT);

  // Try to mount SPIFFS without formatting on failure
  if (!SPIFFS.begin(false)) {
    // If SPIFFS does not work, we wait for serial connection...
    while(!Serial);
    delay(1000);

    // Ask to format SPIFFS using serial interface
    Serial.print("Mounting SPIFFS failed. Try formatting? (y/n): ");
    while(!Serial.available());
    Serial.println();

    // If the user did not accept to try formatting SPIFFS or formatting failed:
    if (Serial.read() != 'y' || !SPIFFS.begin(true)) {
      Serial.println("SPIFFS not available. Stop.");
      while(true);
    }
    Serial.println("SPIFFS has been formated.");
  }
  Serial.println("SPIFFS has been mounted.");

  // Now that SPIFFS is ready, we can create or load the certificate
  SSLCert *cert = getCertificate();
  if (cert == NULL) {
    Serial.println("Could not load certificate. Stop.");
    while(true);
  }

  // Initialize event structure:
  for(int i = 0; i < MAX_EVENTS; i++) {
    events[i].active = false;
    events[i].gpio = 0;
    events[i].state = LOW;
    events[i].time = 0;
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

  // Create the server with the certificate we loaded before
  secureServer = new HTTPSServer(cert);

  // We register the SPIFFS handler as the default node, so every request that does
  // not hit any other node will be redirected to the file system.
  ResourceNode * spiffsNode = new ResourceNode("", "", &handleSPIFFS);
  secureServer->setDefaultNode(spiffsNode);

  // Add a handler that serves the current system uptime at GET /api/uptime
  ResourceNode * uptimeNode = new ResourceNode("/api/uptime", "GET", &handleGetUptime);
  secureServer->registerNode(uptimeNode);

  // Add the handler nodes that deal with modifying the events:
  ResourceNode * getEventsNode = new ResourceNode("/api/events", "GET", &handleGetEvents);
  secureServer->registerNode(getEventsNode);
  ResourceNode * postEventNode = new ResourceNode("/api/events", "POST", &handlePostEvent);
  secureServer->registerNode(postEventNode);
  ResourceNode * deleteEventNode = new ResourceNode("/api/events/*", "DELETE", &handleDeleteEvent);
  secureServer->registerNode(deleteEventNode);

  Serial.println("Starting server...");
  secureServer->start();
  if (secureServer->isRunning()) {
    Serial.println("Server ready.");
  }
}

void loop() {
  // This call will let the server do its work
  secureServer->loop();

  // Here we handle the events
  unsigned long now = millis() / 1000;
  for (int i = 0; i < MAX_EVENTS; i++) {
    // Only handle active events:
    if (events[i].active) {
      // Only if the counter has recently been exceeded
      if (events[i].time < now) {
      // Apply the state change
      digitalWrite(events[i].gpio, events[i].state);

      // Deactivate the event so it doesn't fire again
      events[i].active = false;
      }
    }
  }

  // Other code would go here...
  delay(1);
}

/**
 * This function will either read the certificate and private key from SPIFFS or
 * create a self-signed certificate and write it to SPIFFS for next boot
 */
SSLCert * getCertificate() {
  // Try to open key and cert file to see if they exist
  File keyFile = SPIFFS.open("/key.der");
  File certFile = SPIFFS.open("/cert.der");

  // If now, create them 
  if (!keyFile || !certFile || keyFile.size()==0 || certFile.size()==0) {
    Serial.println("No certificate found in SPIFFS, generating a new one for you.");
    Serial.println("If you face a Guru Meditation, give the script another try (or two...).");
    Serial.println("This may take up to a minute, so please stand by :)");

    SSLCert * newCert = new SSLCert();
    // The part after the CN= is the domain that this certificate will match, in this
    // case, it's esp32.local.
    // However, as the certificate is self-signed, your browser won't trust the server
    // anyway.
    int res = createSelfSignedCert(*newCert, KEYSIZE_1024, "CN=esp32.local,O=acme,C=DE");
    if (res == 0) {
      // We now have a certificate. We store it on the SPIFFS to restore it on next boot.

      bool failure = false;
      // Private key
      keyFile = SPIFFS.open("/key.der", FILE_WRITE);
      if (!keyFile || !keyFile.write(newCert->getPKData(), newCert->getPKLength())) {
        Serial.println("Could not write /key.der");
        failure = true;
      }
      if (keyFile) keyFile.close();

      // Certificate
      certFile = SPIFFS.open("/cert.der", FILE_WRITE);
      if (!certFile || !certFile.write(newCert->getCertData(), newCert->getCertLength())) {
        Serial.println("Could not write /cert.der");
        failure = true;
      }
      if (certFile) certFile.close();

      if (failure) {
        Serial.println("Certificate could not be stored permanently, generating new certificate on reboot...");
      }

      return newCert;

    } else {
      // Certificate generation failed. Inform the user.
      Serial.println("An error occured during certificate generation.");
      Serial.print("Error code is 0x");
      Serial.println(res, HEX);
      Serial.println("You may have a look at SSLCert.h to find the reason for this error.");
      return NULL;
    }

	} else {
    Serial.println("Reading certificate from SPIFFS.");

    // The files exist, so we can create a certificate based on them
    size_t keySize = keyFile.size();
    size_t certSize = certFile.size();

    uint8_t * keyBuffer = new uint8_t[keySize];
    if (keyBuffer == NULL) {
      Serial.println("Not enough memory to load privat key");
      return NULL;
    }
    uint8_t * certBuffer = new uint8_t[certSize];
    if (certBuffer == NULL) {
      delete[] keyBuffer;
      Serial.println("Not enough memory to load certificate");
      return NULL;
    }
    keyFile.read(keyBuffer, keySize);
    certFile.read(certBuffer, certSize);

    // Close the files
    keyFile.close();
    certFile.close();
    Serial.printf("Read %u bytes of certificate and %u bytes of key from SPIFFS\n", certSize, keySize);
    return new SSLCert(certBuffer, certSize, keyBuffer, keySize);
  }
}

/**
 * This handler function will try to load the requested resource from SPIFFS's /public folder.
 * 
 * If the method is not GET, it will throw 405, if the file is not found, it will throw 404.
 */
void handleSPIFFS(HTTPRequest * req, HTTPResponse * res) {
	
  // We only handle GET here
  if (req->getMethod() == "GET") {
    // Redirect / to /index.html
    std::string reqFile = req->getRequestString()=="/" ? "/index.html" : req->getRequestString();

    // Try to open the file
    std::string filename = std::string(DIR_PUBLIC) + reqFile;

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
      if(reqFile.rfind(contentTypes[cTypeIdx][0])!=std::string::npos) {
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
  } else {
    // If there's any body, discard it
    req->discardRequestBody();
    // Send "405 Method not allowed" as response
    res->setStatusCode(405);
    res->setStatusText("Method not allowed");
    res->println("405 Method not allowed");
  }
}

/**
 * This function will return the uptime in seconds as JSON object:
 * {"uptime": 42}
 */
void handleGetUptime(HTTPRequest * req, HTTPResponse * res) {
  // Create a buffer of size 1 (pretty simple, we have just one key here)
  StaticJsonBuffer<JSON_OBJECT_SIZE(1)> jsonBuffer;
  // Create an object at the root
  JsonObject& obj = jsonBuffer.createObject();
  // Set the uptime key to the uptime in seconds
  obj["uptime"] = millis()/1000;
  // Set the content type of the response
  res->setHeader("Content-Type", "application/json");
  // As HTTPResponse implements the Print interface, this works fine. Just remember
  // to use *, as we only have a pointer to the HTTPResponse here:
  obj.printTo(*res);
}

/**
 * This handler will return a JSON array of currently active events for GET /api/events
 */
void handleGetEvents(HTTPRequest * req, HTTPResponse * res) {
  // We need to calculate the capacity of the json buffer
  int activeEvents = 0;
  for(int i = 0; i < MAX_EVENTS; i++) {
    if (events[i].active) activeEvents++;
  }

  // For each active event, we need 1 array element with 4 objects
  const size_t capacity = JSON_ARRAY_SIZE(activeEvents) + activeEvents * JSON_OBJECT_SIZE(4);

  // DynamicJsonBuffer is created on the heap instead of the stack
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonArray& arr = jsonBuffer.createArray();
  for(int i = 0; i < MAX_EVENTS; i++) {
    if (events[i].active) {
      JsonObject& eventObj = arr.createNestedObject();
      eventObj["gpio"] = events[i].gpio;
      eventObj["state"] = events[i].state;
      eventObj["time"] = events[i].time;
      // Add the index to allow delete and post to identify the element
      eventObj["id"] = i;
    }
  }

  // Print to response
  res->setHeader("Content-Type", "application/json");
  arr.printTo(*res);
}

void handlePostEvent(HTTPRequest * req, HTTPResponse * res) {
  // We expect an object with 4 elements and add some buffer
  const size_t capacity = JSON_OBJECT_SIZE(4) + 180;
  DynamicJsonBuffer jsonBuffer(capacity);

  // Create buffer to read request
  char * buffer = new char[capacity + 1];
  memset(buffer, 0, capacity+1);

  // Try to read request into buffer
  size_t idx = 0;
  // while "not everything read" or "buffer is full"
  while (!req->requestComplete() && idx < capacity) {
    idx += req->readChars(buffer + idx, capacity-idx);
  }

  // If the request is still not read completely, we cannot process it.
  if (!req->requestComplete()) {
    res->setStatusCode(413);
    res->setStatusText("Request entity too large");
    res->println("413 Request entity too large");
    // Clean up
    delete[] buffer;
    return;
  }

  // Parse the object
  JsonObject& reqObj = jsonBuffer.parseObject(buffer);

  // Check input data types
  bool dataValid = true;
  if (!reqObj.is<long>("time") || !reqObj.is<int>("gpio") || !reqObj.is<int>("state")) {
    dataValid = false;
  }
	
  // Check actual values
  unsigned long eTime = 0;
  int eGpio = 0;
  int eState = LOW;
  if (dataValid) {
    eTime = reqObj["time"];
    if (eTime < millis()/1000) dataValid = false;

    eGpio = reqObj["gpio"];
    if (!(eGpio == 25 || eGpio == 26 || eGpio == 27 || eGpio == 32 || eGpio == 33)) dataValid = false;

    eState = reqObj["state"];
    if (eState != HIGH && eState != LOW) dataValid = false;
  }

  // Clean up, we don't need the buffer any longer
  delete[] buffer;

  // If something failed: 400
  if (!dataValid) {
    res->setStatusCode(400);
    res->setStatusText("Bad Request");
    res->println("400 Bad Request");
    return;
  }

  // Try to find an inactive event in the list to write the data to
  int eventID = -1;
  for(int i = 0; i < MAX_EVENTS && eventID==-1; i++) {
    if (!events[i].active) {
      eventID = i;
      events[i].gpio = eGpio;
      events[i].time = eTime;
      events[i].state = eState;
      events[i].active = true;
    }
  }

  // Check if we could store the event
  if (eventID>-1) {
    // Create a buffer for the response
    StaticJsonBuffer<JSON_OBJECT_SIZE(4)> resBuffer;

    // Create an object at the root
    JsonObject& resObj = resBuffer.createObject();

    // Set the uptime key to the uptime in seconds
    resObj["gpio"] = events[eventID].gpio;
    resObj["state"] = events[eventID].state;
    resObj["time"] = events[eventID].time;
    resObj["id"] = eventID;

    // Write the response
    res->setHeader("Content-Type", "application/json");
    resObj.printTo(*res);

  } else {
    // We could not store the event, no free slot.
    res->setStatusCode(507);
    res->setStatusText("Insufficient storage");
    res->println("507 Insufficient storage");

  }
}

/**
 * This handler will delete an event (meaning: deactive the event)
 */
void handleDeleteEvent(HTTPRequest * req, HTTPResponse * res) {
  // Access the parameter from the URL. See Parameters example for more details on this
  ResourceParameters * params = req->getParams();
  size_t eid = std::atoi(params->getPathParameter(0).c_str());

  if (eid < MAX_EVENTS) {
    // Set the inactive flag
    events[eid].active = false;
    // And return a successful response without body
    res->setStatusCode(204);
    res->setStatusText("No Content");
  } else {
    // Send error message
    res->setStatusCode(400);
    res->setStatusText("Bad Request");
    res->println("400 Bad Request");
  }
}
