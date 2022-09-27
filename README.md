# ESP32 HTTPS Server

This repository contains an HTTPS server library that can be used with the [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32). It supports HTTP as well.

## Features

- Providing support for HTTP, HTTPS or both at the same time
- Handling requests in callback functions that can be bound to URLs, like for example in Express or Servlets.
- Abstraction of handling the HTTP stuff and providing a simple API for it, eg. to access parameters, headers, HTTP Basic Auth etc.
- Using middleware functions as proxy to every request to perform central tasks like authentication or logging.
- Make use of the built-in encryption of the ESP32 module for HTTPS.
- Handle multiple clients in parallel (max. 3-4 TLS clients due to memory limits).
- Usage of `Connection: keep-alive` and SSL session reuse to reduce the overhead of SSL handshakes and speed up data transfer.

## Dependencies

The library is self-contained and just needs the Arduino and ESP32 system libraries. Running the examples requires the WiFi library in addition to that.

## Setup Instructions

The steps to install this library depend on the IDE you are using. PlatformIO is recommended as you have more options to configure the library (see [Advanced Configuration](#advanced-configuration)), but the library can be used with the plain Arduino IDE.

### PlatformIO (Recommended)

The library is listed in PlatformIO's [library registry](https://platformio.org/lib/show/5887/esp32_https_server/). If you're using the IDE, search for `esp32_https_server` and install it, on the command line, just run:

```bash
pio lib install "esp32_https_server"
```

New release can take one or two days before they get picked up by the library crawler which makes them available in the registry. The version numbers of [releases](https://github.com/fhessel/esp32_https_server/releases) are based on [semantic versioning](https://semver.org/).

Add the library to your `platform.ini` like in the following example:

```ini
[platformio]
env_default = wrover

[env:wrover]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = esp32_https_server
```

If you want a specific version, you can use a notation like `lib_deps = esp32_https_server@0.3.0`. More information on this can be found in the [PlatformIO documentation](https://docs.platformio.org/en/latest/projectconf/section_env_library.html).

To use the current master of the library, don't add it to your `platform.ini` but go to your project's `libs` folder and run the following command to get a copy of repository:

```bash
git clone https://github.com/fhessel/esp32_https_server.git
```

> **Note:** While the `master` branch should contain a running version of the library at any time, the API might already have changes towards the next major release. So for a stable setup, it is recommended to use a published release.

### Arduino IDE

You can install the library using Arduino IDE's [library manager](https://www.arduino.cc/en/Guide/Libraries). In the _Sketch_ menu, click on _Include Library_ and select _Manage Libraries..._ directly at the top of the menu. Then search for "ESP32 HTTPS Server" and click on the _Install_ button.

Alternatively, you can download [a release](https://github.com/fhessel/esp32_https_server/releases) and extract it into your `Arduino/libraries` folder. Then restart your IDE.

If you want to use the latest `master` branch, open a command line, navigate to the libraries folder and run:

```bash
git clone https://github.com/fhessel/esp32_https_server.git
```

> **Note:** While the `master` branch should contain a running version of the library at any time, the API might already have changes towards the next major release. So for a stable setup, it is recommended to use a published release.

## Examples

> **Note:** To run the examples (except for the _Self-Signed-Certificates_ example), you need to execute the script extras/create_cert.sh first (see [Issue #26](https://github.com/fhessel/esp32_https_server/issues/26) for Windows). This script will create a simple CA to sign certificates that are used with the examples. Some notes on the usage can be found in the extras/README.md file.

You will find several examples showing how you can use the library:

- [Static-Page](examples/Static-Page/Static-Page.ino): Short example showing how to serve some static resources with the server. You should start with this sketch and get familiar with it before having a look at the more complex examples.
- [Parameters](examples/Parameters/Parameters.ino): Shows how you can access request parameters (the part after the question mark in the URL) or parameters in dynamic URLs (like /led/1, /led/2, ...)
- [Put-Post-Echo](examples/Put-Post-Echo/Put-Post-Echo.ino): Implements a simple echo service for PUT and POST requests that returns the request body as response body. Also shows how to differentiate between multiple HTTP methods for the same URL.
- [HTTPS-and-HTTP](examples/HTTPS-and-HTTP/HTTPS-and-HTTP.ino): Shows how to serve resources via HTTP and HTTPS in parallel and how to check if the user is using a secure connection during request handling
- [Middleware](examples/Middleware/Middleware.ino): Shows how to use the middleware API for logging. Middleware functions are defined very similar to webservers like Express.
- [Authentication](examples/Authentication/Authentication.ino): Implements a chain of two middleware functions to handle authentication and authorization using HTTP Basic Auth.
- [Async-Server](examples/Async-Server/Async-Server.ino): Like the Static-Page example, but the server runs in a separate task on the ESP32, so you do not need to call the loop() function in your main sketch.
- [Websocket-Chat](examples/Websocket-Chat/Websocket-Chat.ino): Provides a browser-based chat built on top of websockets. **Note:** Websockets are still under development!
- [Parameter-Validation](examples/Parameter-Validation/Parameter-Validation.ino): Shows how you can integrate validator functions to do formal checks on parameters in your URL.
- [Self-Signed-Certificate](examples/Self-Signed-Certificate/Self-Signed-Certificate.ino): Shows how to generate a self-signed certificate on the fly on the ESP when the sketch starts. You do not need to run `create_cert.sh` to use this example.
- [REST-API](examples/REST-API/REST-API.ino): Uses [ArduinoJSON](https://arduinojson.org/) and [SPIFFS file upload](https://github.com/me-no-dev/arduino-esp32fs-plugin) to serve a small web interface that provides a REST API.

If you encounter error messages that cert.h or private\_key.h are missing when running an example, make sure to run create\_cert.sh first (see Setup Instructions).

You might also want to check out the (work-in-progress) [Documentation](https://fhessel.github.io/esp32_https_server/).

## Get Started

The following includes are required to be able to setup the server.

```C++
// Inlcudes for setting up the server
#include <HTTPSServer.hpp>

// Define the certificate data for the server (Certificate and private key)
#include <SSLCert.hpp>

// Includes to define request handler callbacks
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

// Required do define ResourceNodes
#include <ResourceNode.hpp>

// Easier access to the classes of the server
using namespace httpsserver
```
### Create Server Instance

You can create your server instance like shown below:

**For HTTP:**

```C++
HTTPServer myServer = HTTPServer();
```

**For HTTPS:**

```C++
// Create certificate data (see extras/README.md on how to create it)
SSLCert cert = SSLCert(
    crt_DER,     // DER-formatted certificate data
    crt_DER_len, // length of the certificate data
    key_DER,     // private key for that certificate
    key_DER_len  // Length of the private key
);

// Setup the server with default configuration and the
// certificate that has been specified before
HTTPSServer myServer = HTTPSServer(cert);
```

By default, the server will listen on port 443. If you want to change that (or some other options), you can have a look at the optional parameters of the [`HTTPServer`](https://fhessel.github.io/esp32_https_server/classhttpsserver_1_1HTTPServer.html) or [`HTTPSServer`](https://fhessel.github.io/esp32_https_server/classhttpsserver_1_1HTTPSServer.html) constructors.

The only difference between the HTTP and HTTPS version is the certificate which you have to configure. Keep in mind that each opened connection of the TLS-enabled `HTTPSServer` requires additional 40 to 50 kB of heap memory for the TLS connection itself. This has to be considered when increasing `maxConnections`.

### Add Resources to the Server

Every _route_ (or path) that should be accessible on the server has to be configured as a so-called `ResourceNode`. Such a node links a handler function to a specific route (like `/test`) and HTTP method (like `GET`). The handler function could look like this:

```C++
void handleRoot(HTTPRequest * req, HTTPResponse * res) {
	// We want to deliver an HTML page, so we set the content type
	res->setHeader("Content-Type", "text/html");
	// The response implements the Print interface, so you can use it just like
	// you would write to Serial etc.
	res->println("<!DOCTYPE html>");
	res->println("<html>");
	res->println("<head><title>Hello World!</title></head>");
	res->println("<body>");
	res->println("<h1>Hello World!</h1>");
	res->print("<p>... from your ESP32!</p>");
	res->println("</body>");
	res->println("</html>");
}
```

As you can see, the function gets references to the [`HTTPRequest`](https://fhessel.github.io/esp32_https_server/classhttpsserver_1_1HTTPRequest.html) and [`HTTPResponse`](https://fhessel.github.io/esp32_https_server/classhttpsserver_1_1HTTPResponse.html). You can use the request to read headers, parameters, authentication information etc. The response can be used to send data to the client, set headers or HTTP status codes.

Now we need to tell the server which URL should be served by this function. This can be done by creating a [`ResourceNode`](https://fhessel.github.io/esp32_https_server/classhttpsserver_1_1ResourceNode.html) (usually in your `setup()` function).

```C++
ResourceNode * nodeRoot = new ResourceNode("/", "GET", &handleRoot);
```

The first parameter defines the route. It should always start with a slash, and using just a slash like in this example means that the function will be called for requests to the server's root (like https://10.0.x.x/).

The second parameter is the HTTP method, `"GET"` in this case.

Finally, you pass a reference to the request handler function to link it to the route and method.

Now you just need to register the created [`ResourceNode`](https://fhessel.github.io/esp32_https_server/classhttpsserver_1_1ResourceNode.html) at your server:

```C++
myServer.registerNode(nodeRoot);
```

That's everything you need to do for a single web page on your server.

Note that you can define a single [`ResourceNode`](https://fhessel.github.io/esp32_https_server/classhttpsserver_1_1ResourceNode.html) via `HTTPServer::setDefaultNode()`, which will be called if no other node on the server matches. Method and route are ignored in this case. Most examples use this to define a 404-handler, which might be a good idea for most scenarios. In case no default node is specified, the server will return with a small error page if no matching route is found.

### Start the Server

A call to [`HTTPServer::start()`](https://fhessel.github.io/esp32_https_server/classhttpsserver_1_1HTTPServer.html#a1b1b6bce0b52348ca5b5664cf497e039) will start the server so that it is listening on the previously specified port:

```C++
myServer.start();
```

This code usually goes into your `setup()` function. You can use `HTTPServer::isRunning()` to check whether the server started successfully.

By default, you need to pass control to the server explicitly. This is done by calling the [`HTTPServer::loop()`](https://fhessel.github.io/esp32_https_server/classhttpsserver_1_1HTTPServer.html#af8f68f5ff6ad101827bcc52217249fe2) function, which you usually will put into your Arduino sketch's `loop()` function. Once called, the server will first check for incoming connection (up to the maximum connection count that has been defined in the constructor), and then handle every open connection if it has new data on the socket. So your request handler functions will be called during the call to `loop()`. Note that if one of your handler functions is blocking, it will block all other connections as well.

### Running the Server asynchronously

If you want to have the server running in the background (and not calling `loop()` by yourself every few milliseconds), you can make use of the ESP32's task feature and put the whole server in a separate task.

See the [Async-Server example](https://github.com/fhessel/esp32_https_server/tree/master/examples/Async-Server) to see how this can be done.

## Advanced Configuration

This section covers some advanced configuration options that allow you, for example, to customize the build process, but which might require more advanced programming skills and a more sophisticated IDE that just the default Arduino IDE.

### Saving Space by Reducing Functionality

To save program space on the microcontroller, there are some parts of the library that can be disabled during compilation and will then not be a part of your program.

The following flags are currently available:

| Flag                      | Effect
| ------------------------- | ---------------------------
| HTTPS_DISABLE_SELFSIGNING | Removes the code for generating a self-signed certificate at runtime. You will need to provide certificate and private key data from another data source to use the `HTTPSServer`.

Setting these flags requires a build environment that gives you some control of the compiler, as libraries are usually compiled separately, so just doing a `#define HTTPS_SOMETHING` in your sketch will not work.

**Example: Configuration with PlatformIO**

To set these flags in PlatformIO, you can modify your `platformio.ini`. To disable for example the self-signed-certificates part of the library, the file could look like this:

```ini
[env:wroom]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = esp32_https_server
build_flags =
  -DHTTPS_DISABLE_SELFSIGNING
```

Note the `-D` in front of the actual flag name, that passes this flag as a definition to the preprocessor. Multiple flags can be added one per line.

### Configure Logging

The server provides some internal logging, which is activated on level `INFO` by default. This will look like this on your serial console:

```
[HTTPS:I] New connection. SocketFID=55
[HTTPS:I] Request: GET / (FID=55)
[HTTPS:I] Connection closed. Socket FID=55
```

Logging output can also be controlled by using compiler flags. This requires an advanced development environment like explained in [Saving Space by Reducing Functionality](#saving-space-by-reducing-functionality).

There are two parameters that can be configured:

- `HTTPS_LOGLEVEL` defines the log level to use
- `HTTPS_LOGTIMESTAMP` adds a timestamp (based on uptime) to each log entry

| Value of `HTTPS_LOGLEVEL` | Error | Warning | Info | Debug |
| ------------------------- | ----- | ------- | ---- | ----- |
| 0                         |       |         |      |       |
| 1                         |   ✓   |         |      |       |
| 2                         |   ✓   |    ✓    |      |       |
| 3                         |   ✓   |    ✓    |  ✓   |       |
| 4                         |   ✓   |    ✓    |  ✓   |   ✓   |

**Example: Configuration with PlatformIO**

To set these flags in PlatformIO, you can modify your `platformio.ini`. The following entries set the minimum log level to warning and enable timestamps

```ini
[env:wroom]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = esp32_https_server
build_flags =
  -DHTTPS_LOGLEVEL=2
  -DHTTPS_LOGTIMESTAMP
```
