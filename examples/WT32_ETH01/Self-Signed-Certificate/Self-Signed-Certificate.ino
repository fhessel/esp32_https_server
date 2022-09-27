/**
   Example for the WT32_ETH01 HTTP(S) Webserver

   IMPORTANT NOTE:

   This script will install an HTTPS Server on your ESP32 with the following
   functionalities:
    - Show simple page on web server root
    - 404 for everything else

   In contrast to the other examples, the certificate and the private key will be
   generated on the WT32_ETH01, so you do not need to provide them here.
   (this means no need to run create_cert.sh)
*/

//////////////////////////////////////////////////

// For WT32_ETH01
#define DEBUG_ETHERNET_WEBSERVER_PORT       Serial

// Debug Level from 0 to 4
#define _ETHERNET_WEBSERVER_LOGLEVEL_       3

#include <WebServer_WT32_ETH01.h>

// Select the IP address according to your local network
IPAddress myIP(192, 168, 2, 232);
IPAddress myGW(192, 168, 2, 1);
IPAddress mySN(255, 255, 255, 0);

// Google DNS Server IP
IPAddress myDNS(8, 8, 8, 8);

//////////////////////////////////////////////////

// Includes for the server
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

// The HTTPS Server comes in a separate namespace. For easier use, include it here.
using namespace httpsserver;

SSLCert * cert;
HTTPSServer * secureServer;

// Handler functions for the various URLs on the server

void handleRoot(HTTPRequest * req, HTTPResponse * res)
{
  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");

  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Hello World!</title></head>");
  res->println("<body>");
  res->println("<h1>Hello World!</h1>");
  res->print("<p>Your server is running for ");
  // A bit of dynamic data: Show the uptime
  res->print((int)(millis() / 1000), DEC);
  res->println(" seconds.</p>");
  res->println("</body>");
  res->println("</html>");
}

void handle404(HTTPRequest * req, HTTPResponse * res)
{
  // Discard request body, if we received any
  // We do this, as this is the default node and may also server POST/PUT requests
  req->discardRequestBody();

  // Set the response status
  res->setStatusCode(404);
  res->setStatusText("Not Found");

  // Set content type of the response
  res->setHeader("Content-Type", "text/html");

  // Write a tiny HTTP page
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Not Found</title></head>");
  res->println("<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>");
  res->println("</html>");
}

void setup()
{
  // For logging
  Serial.begin(115200);
  delay(3000); // wait for the monitor to reconnect after uploading.

  Serial.print("\nStarting Self_Signed_Certificate on "); Serial.print(ARDUINO_BOARD);
  Serial.println(" with " + String(SHIELD_TYPE));
  Serial.println(WEBSERVER_WT32_ETH01_VERSION);

  Serial.println("Creating a new self-signed certificate.");
  Serial.println("This may take up to a minute, so be patient ;-)");

  // First, we create an empty certificate:
  cert = new SSLCert();

  // Now, we use the function createSelfSignedCert to create private key and certificate.
  // The function takes the following paramters:
  // - Key size: 1024 or 2048 bit should be fine here, 4096 on the ESP might be "paranoid mode"
  //   (in generel: shorter key = faster but less secure)
  // - Distinguished name: The name of the host as used in certificates.
  //   If you want to run your own DNS, the part after CN (Common Name) should match the DNS
  //   entry pointing to your WT32_ETH01. You can try to insert an IP there, but that's not really good style.
  // - Dates for certificate validity (optional, default is 2022-2032, both included)
  //   Format is YYYYMMDDhhmmss
  int createCertResult = createSelfSignedCert(
                           *cert,
                           KEYSIZE_2048,
                           "CN=myesp32.local,O=FancyCompany,C=CA",
                           "20220101000000",
                           "20330101000000"
                         );

  // Now check if creating that worked
  if (createCertResult != 0)
  {
    Serial.printf("Cerating certificate failed. Error Code = 0x%02X, check SSLCert.hpp for details", createCertResult);

    while (true)
      delay(500);
  }
  
  Serial.println("Creating the certificate was successful");

  // If you're working on a serious project, this would be a good place to initialize some form of non-volatile storage
  // and to put the certificate and the key there. This has the advantage that the certificate stays the same after a reboot
  // so your client still trusts your server, additionally you increase the speed-up of your application.
  // Some browsers like Firefox might even reject the second run for the same issuer name (the distinguished name defined above).
  //
  // Storing:
  //   For the key:
  //     cert->getPKLength() will return the length of the private key in byte
  //     cert->getPKData() will return the actual private key (in DER-format, if that matters to you)
  //   For the certificate:
  //     cert->getCertLength() and ->getCertData() do the same for the actual certificate data.
  // Restoring:
  //   When your applications boots, check your non-volatile storage for an existing certificate, and if you find one
  //   use the parameterized SSLCert constructor to re-create the certificate and pass it to the HTTPSServer.
  //
  // A short reminder on key security: If you're working on something professional, be aware that the storage of the ESP32 is
  // not encrypted in any way. This means that if you just write it to the flash storage, it is easy to extract it if someone
  // gets a hand on your hardware. You should decide if that's a relevant risk for you and apply countermeasures like flash
  // encryption if neccessary

  // We can now use the new certificate to setup our server as usual.
  secureServer = new HTTPSServer(cert);

  ///////////////////////////////////////////////

  // To be called before ETH.begin()
  WT32_ETH01_onEvent();

  //bool begin(uint8_t phy_addr=ETH_PHY_ADDR, int power=ETH_PHY_POWER, int mdc=ETH_PHY_MDC, int mdio=ETH_PHY_MDIO,
  //           eth_phy_type_t type=ETH_PHY_TYPE, eth_clock_mode_t clk_mode=ETH_CLK_MODE);
  //ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_TYPE, ETH_CLK_MODE);
  ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);

  // Static IP, leave without this line to get IP via DHCP
  //bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = 0, IPAddress dns2 = 0);
  ETH.config(myIP, myGW, mySN, myDNS);

  WT32_ETH01_waitForConnect();

  Serial.print(F("HTTP EthernetWebServer is @ IP : "));
  Serial.println(ETH.localIP());

  ///////////////////////////////////////////////

  // For every resource available on the server, we need to create a ResourceNode
  // The ResourceNode links URL and HTTP method to a handler function
  ResourceNode * nodeRoot    = new ResourceNode("/", "GET", &handleRoot);
  ResourceNode * node404     = new ResourceNode("", "GET", &handle404);

  // Add the root node to the server
  secureServer->registerNode(nodeRoot);
  // Add the 404 not found node to the server.
  secureServer->setDefaultNode(node404);

  Serial.println("Starting server...");
  secureServer->start();

  if (secureServer->isRunning())
  {
    Serial.println("Server ready.");
  }
}

void loop()
{
  // This call will let the server do its work
  secureServer->loop();

  // Other code would go here...
  delay(1);
}
