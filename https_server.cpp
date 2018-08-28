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
	res->println("<p>... from your ESP32</p>");
	// The image resource is created in the awesomeCallback some lines below
	res->println("<img src=\"images/awesome.svg\" alt=\"Awesome face\" style=\"width:250px;\" />");
	res->println("<img src=\"images/awesome.svg?color=de58fe\" alt=\"Awesome face\" style=\"width:250px;\" />");
	res->println("<img src=\"images/awesome.svg?color=58fede\" alt=\"Awesome face\" style=\"width:250px;\" />");
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
 * The URL Param Callback demonstrates the usage of placeholders in the URL.
 *
 * This callback function is mapped to "param/ * / *" (ignore the spaces, they are required
 * because of the C comment syntax).
 *
 * The placeholder values can be accessed through HTTPRequest::getParams. They are indexed
 * beginning from 0.
 */
void urlParamCallback(HTTPRequest * req, HTTPResponse * res) {
	// Get access to the parameters
	ResourceParameters * params = req->getParams();

	// Set a simple content type
	res->setHeader("Content-Type", "text/plain");

	// Print the first parameter
	res->print("Parameter 1: ");
	res->printStd(params->getUrlParameter(0));

	// Print the second parameter
	res->print("\nParameter 2: ");
	res->printStd(params->getUrlParameter(1));
}

/**
 * This callback responds with an SVG image to a GET request. The icon is the awesome smiley.
 *
 * If the color request parameter is set (so the URL is like awesome.svg?color=fede58), the
 * background of our awesome face is changed.
 */
void awesomeCallback(HTTPRequest * req, HTTPResponse * res) {
	// Get access to the parameters
	ResourceParameters * params = req->getParams();

	// Set SVG content type
	res->setHeader("Content-Type", "image/svg+xml");

	// Check if there is a suitabel fill color in the parameter:
	std::string fillColor = "fede58";

	// Get request parameter
	std::string colorParamName = "color";
	if (params->isRequestParameterSet(colorParamName)) {
		std::string requestColor = params->getRequestParameter(colorParamName);
		if (requestColor.length()==6) {
			bool colorOk = true;
			for(int i = 1; i < 6 && colorOk; i++) {
				if (!(
						(requestColor[i]>='0' && requestColor[i]<='9' ) ||
						(requestColor[i]>='a' && requestColor[i]<='f' )
				)) {
					colorOk = false;
				}
			}
			if (colorOk) {
				fillColor = requestColor;
			}
		}
	}

	// Print the data
	// Source: https://commons.wikimedia.org/wiki/File:718smiley.svg
	res->print("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>");
	res->print("<svg id=\"svg1923\" width=\"733\" xmlns=\"http://www.w3.org/2000/svg\" height=\"733\">");
	res->print("<circle cy=\"366.5\" cx=\"366.5\" r=\"366.5\"/>");
	res->print("<circle cy=\"366.5\" cx=\"366.5\" r=\"336.5\" fill=\"#");
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

/**
 * This callback is configured to match all OPTIONS requests (see pattern configuration below)
 *
 * This allows to define headers there that are required to allow cross-domain-xhr-requests,
 * which enabled a REST-API that can be used on the esp32, while the WebInterface is hosted
 * somewhere else (on a host with more storage space to provide huge JS libraries etc.)
 *
 * An example use case would be an IoT dashboard that connects to a bunch of local esp32s,
 * which provide data via their REST-interfaces that is aggregated in the dashboard.
 */
void corsCallback(HTTPRequest * req, HTTPResponse * res) {
	res->setHeader("Access-Control-Allow-Methods", "HEAD,GET,POST,DELETE,PUT,OPTIONS");
	res->setHeader("Access-Control-Allow-Origin",  "*");
	res->setHeader("Access-Control-Allow-Headers", "*");
}

/**
 * This callback simply copies the requests body into the response body.
 *
 * It can be used to test POST and PUT functionality and is configured to reply to
 * POST /echo and PUT /echo
 */
void echoCallback(HTTPRequest * req, HTTPResponse * res) {
	res->setHeader("Content-Type","text/plain");
	byte buffer[256];
	while(!(req->requestComplete())) {
		size_t s = req->readBytes(buffer, 256);
		res->write(buffer, s);
	}
}

/**
 * This callback will be registered as default callback. The default callback is used
 * if no other node matches the request.
 *
 * Again, another content type is shown (json).
 */
void notfoundCallback(HTTPRequest * req, HTTPResponse * res) {
	// Discard the request body, as the 404-handler may also be used for put and post actions
	req->discardRequestBody();
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
	Serial.print(" connected. IP=");
	Serial.println(WiFi.localIP());

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
	// If stack canary errors occur, try to increase the stack size (3rd parameter)
	// or to put as much stuff as possible onto the heap (ResourceNodes etc)
	xTaskCreatePinnedToCore(serverTask, "https443", 4096, NULL, 1, NULL, ARDUINO_RUNNING_CORE);

	Serial.println("Beginning to loop()...");
}

// The loop function is called in an endless loop
void loop() {
	// Use your normal loop without thinking of the server in the background

	// Delay for about five seconds and print some message on the Serial console.
	delay(5 * 1000);

	// We use this loop only to show memory usage for debugging purposes
	uint32_t freeheap = ESP.getFreeHeap();
	Serial.printf("Free Heap: %9d \n", freeheap);
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
			example_crt_DER, example_crt_DER_len,
			example_key_DER, example_key_DER_len
	);

	// The faviconCallback now is assigned to the /favicon.ico node, when accessed by GET
	// This means, it can be accessed by opening https://myesp/favicon.ico in all
	// web browsers. Most browser fetch this file in background every time a new webserver
	// is used to show the icon in the tab of that website.
	ResourceNode * faviconNode   = new ResourceNode("/favicon.ico", "GET", &faviconCallback);

	// The awesomeCallback is very similar to the favicon.
	ResourceNode * awesomeNode   = new ResourceNode("/images/awesome.svg", "GET", &awesomeCallback);

	// A simple callback showing URL parameters. Every asterisk (*) is a placeholder value
	// So, the following URL has two placeholders that have to be filled.
	// This is especially useful for REST-APIs where you want to represent an object ID in the
	// url. Placeholders are arbitrary strings, but may be converted to integers (Error handling
	// is up to the callback, eg. returning 404 if there is no suitable resource for that placeholder
	// value)
	ResourceNode * urlParamNode  = new ResourceNode("/param/*/*", "GET", &urlParamCallback);

	// The echoCallback is configured on the path /echo for POST and PUT requests. It just copies request
	// body to response body. To enable it for both methods, two nodes have to be created:
	ResourceNode * echoNodePost  = new ResourceNode("/echo", "POST", &echoCallback);
	ResourceNode * echoNodePut   = new ResourceNode("/echo", "PUT",  &echoCallback);

	// The root node (on GET /) will be called when no directory on the server is specified in
	// the request, so this node can be accessed through https://myesp/
	ResourceNode * rootNode      = new ResourceNode("/", "GET", &testCallback);

	// As mentioned above, we want to answer all OPTIONS requests with a response that allows
	// cross-domain XHR. To do so, we bind the corsCallback to match all options request
	// (we can exploit the asterisk functionality for this. The callback is not required to
	// process the parameters in any way.)
	// Note the difference to the "/" in the rootNode above - "/" matches ONLY that specific
	// resource, while slash and asterisk is more or less provides a catch all behavior
	ResourceNode * corsNode      = new ResourceNode("/*", "OPTIONS", &corsCallback);

	// The not found node will be used when no other node matches, and it's configured as
	// defaultNode in the server.
	// Note: Despite resource and method string have to be specified when a node is created,
	// they are ignored for the default node. However, this makes it possible to register another
	// node as default node as well.
	ResourceNode * notFoundNode  = new ResourceNode("/", "GET", &notfoundCallback);

	// Create the server. The constructor takes some optional parameters, eg. to specify the TCP
	// port that should be used. However, defining a certificate is mandatory.
	HTTPSServer server = HTTPSServer(&cert);

	// Register the nodes that have been configured on the web server.
	server.setDefaultNode(notFoundNode);
	server.registerNode(rootNode);
	server.registerNode(faviconNode);
	server.registerNode(awesomeNode);
	server.registerNode(urlParamNode);
	server.registerNode(echoNodePost);
	server.registerNode(echoNodePut);
	server.registerNode(corsNode);

	// Add a default header to the server that will be added to every response. In this example, we
	// use it only for adding the server name, but it could also be used to add CORS-headers to every response
	server.setDefaultHeader("Server", "esp32-http-server");

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
