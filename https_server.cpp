// Do not remove the include below
#include "https_server.h"

using namespace httpsserver;

void testCallback(HTTPRequest * req, HTTPResponse * res) {
	res->setStatusCode(200);
	res->setStatusText("OK");
	res->setHeader("Content-Type", "text/html; charset=utf8");

	res->print("<!DOCTYPE html>");
	res->print("<html>");
	res->print("<head>");
	res->print("<title>HTTPS Server on ESP32</title>");
	res->print("</head>");
	res->print("<body>");
	res->print("<h1>Hello world!</h1>");
	res->print("<p>From your ESP32</p>");
	res->print("</body>");
	res->print("</html>");
}

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

	Serial.print("Connecting WiFi");

    WiFi.begin(WIFI_SSID, WIFI_PSK);
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(100);
	}
	Serial.println(" connected.");

	Serial.println("Creating server task... ");
	xTaskCreatePinnedToCore(serverTask, "https443", 4096, NULL, 1, NULL, ARDUINO_RUNNING_CORE);

	Serial.println("Beginning to loop()...");
}

// The loop function is called in an endless loop
void loop() {
	// Use your normal loop without thinking of the server in the background
	Serial.println("Hello from main loop!");
	delay(30*1000);
}

void serverTask(void *params) {
	Serial.println("Configuring Server...");

	// Define the certificate that should be used
	SSLCert cert = SSLCert(
			pariot_crt_DER, pariot_crt_DER_len,
			pariot_key_DER, pariot_key_DER_len
	);

	// Create some example nodes
	ResourceNode rootNode = ResourceNode("/", "GET", &testCallback);
	ResourceNode notFoundNode = ResourceNode("/", "GET", &notfoundCallback);

	// Create the server
	HTTPSServer server = HTTPSServer(&cert);

	// Register the nodes
	server.setDefaultNode(&notFoundNode);
	server.registerNode(&rootNode);

	Serial.println("Starting Server...");
	server.start();

	if (server.isRunning()) {
		Serial.println("Server started.");
		while(1) {
			// Run the server loop
			server.loop();
			delay(1);
		}
	} else {
		Serial.println("Starting Server FAILED! Restart in 10 seconds");
		delay(10000);
		ESP.restart();
	}

}
