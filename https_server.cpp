// Do not remove the include below
#include "https_server.h"

using namespace httpsserver;

/**
 * This callback will be linked to the web server root and answer with
 * a small HTML page that shows the uptime of the esp.
 *
 * Note that the headers may be transmitted as soon as the first data output
 * is sent. So calls to setStatusCode, setStatusText or setHeader should be
 * made before the print functions are called.
 *
 * HTTPResponse implements Print, so it can be used very similar to other
 * Arduino classes like Serial/...
 */
void testCallback(HTTPRequest * req, HTTPResponse * res) {
	res->setStatusCode(200);
	res->setStatusText("OK");
	res->setHeader("Content-Type", "text/html; charset=utf8");

	res->println("<!DOCTYPE html>");
	res->println("<html>");
	res->println("<head>");
	res->println("<title>HTTPS Server on ESP32</title>");
	res->println("</head>");
	res->println("<body>");
	res->println("<h1>Hello world!</h1>");
	res->println("<p>From your ESP32</p>");
	res->print("<p>System has been up for ");
	res->print((int)(millis()/1000), DEC);
	res->println(" seconds.</p>");
	res->println("</body>");
	res->println("</html>");
}

/**
 * Binary data is also supported. We use it to provide a fancy
 * favicon at /favicon.ico.
 *
 * Again, calls to setHeader etc. have to be made before using
 * write or print.
 */
void faviconCallback(HTTPRequest * req, HTTPResponse * res) {
	res->setHeader("Content-Type", "image/vnd.microsoft.icon");
	res->write(FAVICON_DATA, FAVICON_LENGTH);
}

/**
 * This callback will be registered as default callback. The default callback is used
 * if no other node matches the request.
 *
 * Again, another content type is shown (json).
 */
void notfoundCallback(HTTPRequest * req, HTTPResponse * res) {
	res->setStatusCode(404);
	res->setStatusText("Not found");
	res->setHeader("Content-Type", "application/json");

	res->print("{\"error\":\"not found\", \"code\":404}");
}



//The setup function is called once at startup of the sketch
void setup()
{
	Serial.begin(115200);
	Serial.println("setup()");

	// Setup wifi. We use the configuration stored at data/wifi/wifi.h
	// If you don't have that file, make sure to copy the wifi.example.h,
	// rename it and also configure your wifi settings there.
	Serial.print("Connecting WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PSK);
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(100);
	}
	Serial.println(" connected.");

	// Setup the server as a separate task.
	//
	// Important note: If the server is launched as a different task, it has its own
	// stack. This means that we cannot use globally instantiated Objects there.
	// -> Make sure to create Server, ResourceNodes, etc. in the function where they
	// are used (serverTask() in this case).
	// Another alternative would be to instantiate the objects on the heap. This is
	// especially important for data that should be accessed by the main thread and
	// the server.
	Serial.println("Creating server task... ");
	xTaskCreatePinnedToCore(serverTask, "https443", 4096, NULL, 1, NULL, ARDUINO_RUNNING_CORE);

	Serial.println("Beginning to loop()...");
}

// The loop function is called in an endless loop
void loop() {
	// Use your normal loop without thinking of the server in the background
	Serial.println("Hello from main loop!");

	// Delay for about half a minute and print some message on the Serial console.
	delay(30*1000);
}

/**
 * As mentioned above, the serverTask method contains the code to start the server.
 *
 * The infinite loop in the function is the equivalent for the loop() function of a
 * regular Arduino sketch.
 */
void serverTask(void *params) {
	Serial.println("Configuring Server...");

	// Define the certificate that should be used
	// See files in tools/cert on how to create the headers containing the certificate.
	// Because they are just byte arrays, it would also be possible to store and load them from
	// non-volatile memory after creating them on the fly when the device is launched for the
	// first time.
	SSLCert cert = SSLCert(
			pariot_crt_DER, pariot_crt_DER_len,
			pariot_key_DER, pariot_key_DER_len
	);

	// The faviconCallback now is assigned to the /favicon.ico node, when accessed by GET
	// This means, it can be accessed by opening https://myesp/favicon.ico in all
	// web browsers. Most browser fetch this file in background every time a new webserver
	// is used to show the icon in the tab of that website.
	ResourceNode faviconNode  = ResourceNode("/favicon.ico", "GET", &faviconCallback);

	// The root node (on GET /) will be called when no directory on the server is specified in
	// the request, so this node can be accessed through https://myesp/
	ResourceNode rootNode     = ResourceNode("/", "GET", &testCallback);

	// The not found node will be used when no other node matches, and it's configured as
	// defaultNode in the server.
	// Note: Despite resource and method string have to be specified when a node is created,
	// they are ignored for the default node. However, this makes it possible to register another
	// node as default node as well.
	ResourceNode notFoundNode = ResourceNode("/", "GET", &notfoundCallback);

	// Create the server. The constructor takes some optional parameters, eg. to specify the TCP
	// port that should be used. However, defining a certificate is mandatory.
	HTTPSServer server = HTTPSServer(&cert);

	// Register the nodes that have been configured on the web server.
	server.setDefaultNode(&notFoundNode);
	server.registerNode(&rootNode);
	server.registerNode(&faviconNode);

	// The web server can be start()ed and stop()ed. When it's stopped, it will close its server port and
	// all open connections and free the resources. Theoretically, it should be possible to run multiple
	// web servers in parallel, however, there might be some restrictions im memory.
	Serial.println("Starting Server...");
	server.start();

	// We check whether the server did come up correctly (it might fail if there aren't enough free resources)
	if (server.isRunning()) {

		// If the server is started, we go into our task's loop
		Serial.println("Server started.");
		while(1) {
			// Run the server loop.
			// This loop function accepts new clients on the server socket if there are connection slots available
			// (see the optional parameter maxConnections on the HTTPSServer constructor).
			// It also frees resources by connections that have been closed by either the client or the application.
			// Finally, it calls the loop() function of each active HTTPSConnection() to process incoming requests,
			// which will finally trigger calls to the request handler callbacks that have been configured through
			// the ResourceNodes.
			server.loop();
			delay(1);
		}
	} else {
		// For the sake of this example, we just restart the ESP in case of failure and hope it's getting better
		// next time.
		Serial.println("Starting Server FAILED! Restart in 10 seconds");
		delay(10000);
		ESP.restart();
	}

}
