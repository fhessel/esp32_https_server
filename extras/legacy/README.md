# ESP32 HTTPS Server

**THIS IS AN OLD VERSION OF THE README / EXAMPLE SKETCH** that was valid before
the repository has been converted to follow the Arduino Library structure.

This repository contains an HTTPS server implementation that can be used with the [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32).

The main goals for the server are:
- Ability to handle multiple clients in parallel (works, but memory limits it to 3-4 clients)
- Possibility to be executed in a task without interrupting the main program (works)
- Ability to handle `Connection: keep-alive` (works), SSL Session Reuse (works) and WebSockets (tbd, see [Issue 13](https://github.com/fhessel/esp32_https_server/issues/9)) to reduce the SSL-overhead for streaming data operations
- Provide a simple user interface for request handling (works) and middleware functionality (tbd, see [Issue 10](https://github.com/fhessel/esp32_https_server/issues/10))

For the future, the project structure will be converted to serve as a regular Arduino Library. For the moment, you may just
copy the https folder into your project, it is self-contained and needs only the WiFi-Library.

## Setup instructions

You have two options how you can use this repository:

- Run the server with some example code showing its functionality (probably a good start)
- Running the server with your own code

### Running the example

The repository currently contains the HTTPS library as well as a sample scripts that shows how the server can be set up and used. The project has been developed using [Sloeber](http://eclipse.baeyens.it/), so it should be possible to import the repository folder as a project there, if you want to run the example.

Furthermore, you need to do some setup after cloning the repository:

- The example needs to connect to your local Wifi and therefor need credentials. For convenience, `data/wifi/wifi.example.h` contains a code skeleton for that. Just copy it to `/data/wifi/wifi.h` and enter your SSID and WPA2 PSK.
- Besides encryption, one main reason to use HTTP with TLS is to authenticate the server. So the server needs a certificate. If you just want to try the code, the repository contains a script to create a basic self-signed certificate. Note that this will be shown as insecure in your browser and you need parameters to allow such certificates if you access your server using command line tools like `openssl s_client`.
You can run `tools/cert/create_cert.sh` to create header files with certificate data in `data/cert/`. In a real-world setup, you should create those certificates more carefully.

The sketch in `https_server.cpp` shows by example how to use various functions of the library. After compiling and flashing it to your ESP32 module, you should be able to access the following resources on the ESP32 via HTTPS:

- your-ip/ - Welcome page showing the uptime and some parameterized SVG-images
- your-ip/favicon.ico - Favicon that is stored in data/favicon.h
- /images/awesome.svg?color=de58fe - SVG-Image, using the optional parameter as background color
- /echo - Will return request body for POST and PUT requests
- /param/[a]/[b] - Will show the parsed parameters a and b
- Any OPTIONS request - Will return some CORS-headers
- Everything else - JSON error object

### Just use the library for your own project

The library is self-contained, so you should be fine by just copying the `https/` folder into your project. Include it into your sketch as follows:

```C++
// Inlcudes for setting up the server
#include "https/HTTPSServer.hpp"

// Includes to define request handler callbacks
#include "https/HTTPRequest.hpp"
#include "https/HTTPResponse.hpp"

// Easier access to the classes of the server
using namespace httpsserver
```

For details on the API, refer to the following chapter.

## Usage

While there are many classes in the `https` folder, you will most likely only need to use three of them directly.

In general, you will set up an **HTTPS server** that is listening on a specific port and handling incoming connections. To tell the server what it should do if it is hit by a request, you will define **request-handler-functions** that are bound to specific URLs. The server will parse the HTTP request, retrieve the matching handler function (if one has been registered for that URL) and than call that function.

### Setting up the server

The minimal code to start the server could look like this:

```C++
// Include the server class to your sketch
#include "https/HTTPSServer.hpp"

// Use the HTTPS server namespace
using namespace httpsserver;

// Create certificate data (see tools/cert on how to create it)
SSLCert cert = SSLCert(
    crt_DER,     // DER-formatted certificate data
    crt_DER_len, // length of the certificate data
    key_DER,     // private key for that certificate
    key_DER_len  // Length of the private key
);

// Setup the server with default configuration and the
// certificate that has been specified before
HTTPSServer myServer = HTTPSServer(cert);

// (Configure request-handler-functions here)

// Open socket and start processing
myServer.start();

```

In a basic setup, you also need to pass control to the server regularly so that it can handle incoming connections or data. To do so, add the following line to your `loop()` function:

```C++
myServer.loop();
```

That's basically all that you need to do to have a server listening on port 443 and handling incoming requests. To add actual functionality, you need also to define handler functions to answer to requests.

#### HTTPSServer

This is a short overview on the public API of the `HTTPSServer` class.

**Constructor**

Creates a new server.

```C++
HTTPSServer(SSLCert * cert, const uint16_t port = 443, const uint8_t maxConnections = 4, const in_addr_t bindAddress = 0)
```

- _cert_ is the certificate data (instance of `SSLCert`) that will be used by the server to authenticate itself
- _port_ is the port that the server will listen on. Defaults to 443
- _maxConnections_ is the maximum amount of incoming connections that will be served simultaneously. Due to memory limitations, four are the maximum that the ESP will be able to handle, and depending on your code it might be required to lower this number. If more connections reach the server at the same time, the server will only `accept` as many as configured and let other connections wait until resources are freed or a client-side timeout occurs.
- _bindAddress_ is the network interface address that the server should bind to. Defaults to 0 = all interfaces. Possible use case: The ESP is simultaneously in AP mode and STA mode, but the server should only be available in one network.

**loop()**

Processes requests, needs to be called periodically.

```C++
loop()
```

This function first checks for new incoming connections and accepts them if there are resources available (see maxConnections parameter of the constructor). Then, control is passed to the open connections to process received data or send data to the client. If a matching handler-function is defined, it will be called now.

To decouple the server loop from your actual application, you can run it in a separate task on your ESP32. See below for an example on how to do it.

**start()**

Starts the server.

```C++
server.start()
```

This function will start the server. It will create the socket and SSL resource, tie everything together and open the port defined in the constructor call on the interface defined in the constructor call.

**stop()**

Stops the server. 

```C++
server.stop()
```

Will try to close every open connection to the server, and then shut the server down and free socket and SSL resources. The call is blocking until every connection has been teared down successfully. You can start the server again afterwards.

**isRunning()**

Returns `true` if the server is running. If you called `start()` and this function returns `false` afterwards, the server could not be started.

```C++
server.isRunning()
```

**setDefaultHeader()**

Adds a HTTP header that will be set in every response.

```C++
server.setDefaultHeader(std::string name, std::string value)
```

- _name_ The name of the header that should be set
- _value_ The value that this header should have

An example use case would be to add [CORS](https://en.wikipedia.org/wiki/Cross-origin_resource_sharing) headers to every request.

Note that you can also set headers in handler functions dynamically.

**registerNode()**

Adds a resource URL to the server.

```C++
server.registerNode(ResourceNode *node)
```

- _node_ The node that should be made available on the server

The specified node will be made available on the server, so that the linked handler function will be called, if the URL specified in the node is requested.

Note that URL matching is done in the order that the nodes are added to the server. This becomes important if you use wildcard-URLs. If you for example want to have the URLs `/items/new` and `/items/*` (with * being an ID), you'll have to register the non-wildcard node first (and the wildcard-node will never get `*=new` as parameter value).

**unregisterNode()**

Removes a resource URL from the server

```C++
server.unregisterNode(ResourceNode *node)
```

- _node_ The node that should no longer be available

The resource is removed from the server so that it can no longer be accessed by requesting the specified resource URL. Note that you have to pass the exact same instance of `ResourceNode` that you also passed to `registerNode()`, otherwise the node cannot be removed.

**setDefaultNode()**

Registers a node as the default node.

```C++
server.setDefaultNode(ResourceNode *node)
```

- _node_ The `ResourceNode` that should act as default node.

The default node is called when no other node matches the requested URL. This is a good way to implement a 404-page. The URL that is configured in the `ResourceNode` is ignored when it is configured as default node.

#### Running the Server in Background

(tbd, see the sample code in https_server.cpp and https_server.h for reference)

### Defining Request-Handler-Functions

Request-handler-functions add actual functionality to the server. They have the following signature:

```C++
handlerFunction(HTTPRequest * req, HTTPResponse * res)
```

They are called once the request line (URL and HTTP method) and the HTTP headers are received completely and can handle an optional request body. They get two parameters:

- _req_ The request data. This object allows access to the request headers, method, URL, URL parameters and the request body.
- _res_ The response data. This object allows to write to the response body, set response headers and the status code.

To define when a specific function should be called, it has to be bound with a URL (pattern) and an HTTP method to form a `ResourceNode`. This node can than be passed to the `HTTPSServer`'s `registerNode()` function to become available. One handler function may be bound to multiple `ResourceNode`s to serve various URLs.

#### ResourceNode

The `ResourceNode` connects a handler function, a request URL and an HTTP method.

**Constructor**

The constructor creates a (immutable) `ResourceNode`:

```C++
ResourceNode(const std::string path, const std::string method, const HTTPSCallbackFunction * callback)
```

- _path_ The request URL that the node should match, for example `/about` to be available under `https://10.0.0.10/about`. May also contain wildcards for some parts of the path, like `/item/*` to match `/item/1` or `/item/abc`. Currently, only full path segments are supported for the `*` wildcard (so you can do `item/*/*` but not `item/a-*` and `item/b-*`).
- _method_ The HTTP method, like `GET` or `POST`. The server does not check if you use valid HTTP methods, so if you really need it, you can get creative here.
- _callback_ The handler function, with a signature as mentioned above.

Request parameters (`/items?sort=ASC`) are allowed for every URL and are made available to the handler function.

See `HTTPRequest` on how to access the wildcard/request parameters.

#### HTTPRequest

The `HTTPRequest` object allows to access request information from within the handler function.

**getHeader()**

Returns the value of an HTTP request header, if it is set

```C++
req.getHeader(std::string name);
```

- _name_ The name of the header

**readChars()**

Reads the request body (or a part of it, if it is not fully received yet) into a char buffer. Useful if you expect textual requests (like JSON data).

```C++
readChars(char * buffer, size_t length);
```

- _buffer_ The buffer to read into (starting at index 0)
- _length_ The length of the buffer
- Returns the length that has actually been read into that buffer

**readBytes**

Like `readChars()` but with an byte array (if you expect binary data).

```C++
size_t readBytes(byte * buffer, size_t length);
```

(see `readChars()`)

**getContentLength()**

Returns the request body length.

```C++
getContentLength();
```

- Returns the length of the request body

**requestComplete()**

Checks whether the whole request body has been read.

```C++
requestComplete();
```

- Returns true, if the whole request body has been read.

Note that every request-handler-function should read the whole body of the request. If it does not, a warning will be logged by the server, as not parsing the whole body is probably a runtime error.

**discardRequestBody**

Drop the request body without parsing it.

```C++
discardRequestBody();
```

This will just push the whole request body to the trash. It can be used to discard the body if you did not expect a body to be present but one has been received (misbehaving client).

**getParams()**

Returns the resource parameters for the resource node.

```C++
getParams();
```

- Returns a `ResourceParameters` instance that allows access to the values of wildcards in a URL or request parameters. See below for details.

#### ResourceParamters

This object can be accessed via the `HTTPRequest` and allows to retrieve parameter values from the URL. _Request parameters_ are the optional parameters following the question mark in a URL (accessed by name), _URL parameters_ are the wildcard parameters defined when creating a resource node (accessed by index).

**isRequestParameterSet()**

Check whether a request parameter is set.

```C++
isRequestParameterSet(std::string &name);
```

- Returns true if the parameter is set.

**getRequestParameter() / getRequestParameterInt()**

Returns the value of a request parameter.

```C++
getRequestParameter(std::string &name);
getRequestParameterInt(std::string &name);
```
- _name_ The name of the parameter to retrieve

The second function is just for convenience and does string to integer conversion for you.

**getUrlParameter() / getUrlParameterInt()**

Returns the value of a URL parameter.

```C++
getUrlParameter(uint8_t idx);
getUrlParameterInt(uint8_t idx);
```

- _idx_ The index of the request parameter, starting with 0

The second function is just for convenience and does string to integer conversion for you.

#### HTTPResponse

The response object allows to send data back to the client.

It also implements Arduino's `Print` interface, so you can use it very similar to well-known classes like `Serial` etc. to write to the request body.

**setStatusCode() / setStatusText**

Allows to set the HTTP status information.

```C++
setStatusCode(uint16_t statusCode);
```

- _statusCode_ The code to be returned, like `404`

```C++
setStatusText(std::string statusText);
```

- _statusText_ The text to be returned, like "Not found"

You also need to write this to the body if you want it to be shown in the browser.

**setHeader()**

Sets a header for this response.

```C++
void setHeader(std::string name, std::string value);
```

- _name_ the name of the header field, like `Content-Type`
- _value_ the value for that header field, like `application/json`

Multiple headers with the same name are not supported. You may concatenate multiple values with a semicolon and pass them to this function once, though.

**isHeaderWritten()**

Checks whether the header has already been sent to the client

```C++
isHeaderWritten();
```

Returns true if the header or parts of it have been sent to the client. Modification of the header values or the HTTP status is no longer possible then.
