# Changelog

## [pending (master)](https://github.com/fhessel/esp32_https_server/tree/master)

New functionality:

–

Bug fixes:

–

Breaking changes:

–

## [v1.0.0](https://github.com/fhessel/esp32_https_server/releases/tag/v1.0.0)

New functionality:

* #78: Control keep-alive behavior by calling `res->setHeader("Connection", "close")`
* #77: Normalize header names
* #72: Support for body parsers. See the [HTML Forms example](examples/HTML-Forms/HTML-Forms.ino) for usage
* #71: `HTTPRequest` supports `getClientIP()`
* #65: New interface to request and path parameters. Allows iterating over path parameters to retrive multi-value parameters

Bug fixes:

* #69: Treat `+` correctly during URL decoding

Breaking changes:

* HTTP header names are normalized for requests and responses: Each header name starts with a capital letter, all letters after a non-printable character are also capitalized, all other letters are converted to lower case.
   * If you want to retrieve header values:
      * Use `Connection` instead of `connection`
      * Use `Content-Type` instead of `CONTENT-TYPE`
* Interface to Request and URL parameters has changed
   * Former *request paremters* are now called *query parameters*
      * `ResourceParameters::getRequestParameterInt()` does no longer exist. Use `std::atoi(str.c_str())`
      * `ResourceParameter::isRequestParameterSet()` is now `ResourceParameter::isQueryParameterSet()`
      * `std::string ResourceParameter::getRequestParameter(std::string const &name)` is replaced by `bool ResourceParameter::getQueryParameter(std::string const &name, std::string &val)`
         * The return value reflects whether a request parameter was specified. The request parameter value is written to the second argument.
         * See issue #62 for mor details
      * `getQueryParameter()` will return the first occurence of a parameter with a given name (that did not change). You can access additional parameter values using the new `ResourceParameters::beginQueryParameters()` and `ResourceParameters::endQueryParameters()` functions (for a URL like `/?foo=bar&foo=baz` this will yield `(foo, bar)` and `(foo, baz)`.
   * Former *URL parameters* are now called *path parameters*
      * `ResourceParameters::getUrlParameterInt()` does no longer exist. Use `std::atoi(str._cstr())`
      * `std::string ResourceParameter::getURLParameter(std::string const &name)` is now `std::string ResourceParameter::getPathParameter(std::string const &name)`
      * `uint8_t HTTPNode::getUrlParamCount()` is now `size_t HTTPNode::getPathParamCount()`

## [v0.3.1](https://github.com/fhessel/esp32_https_server/releases/tag/v0.3.1)

Bug Fixes:

- Issues regarding websockets (#58, #59)

## [v0.3.0](https://github.com/fhessel/esp32_https_server/releases/tag/v0.3.0)

New "Features" and other changes:

* Add HTTP Status Code 431 - Header too long
* Added doxgen documentation
* Some Typos
* Allow modification of HTTPSServerConstants using CFLAGS
* Configurable logging
* Use spaces everywhere

Bug Fixes:

* Clean close connections correctly when stopping the server
* Fix off-by-one error in number formatting (e.g. content-length)
* Close WebSockets properly when shutting down the server.
* Handle percent encoding of URL and request parameters.
* WebSocket example: Hostname instead of IP

Breaking Changes:

* (internal) HTTPConnection::clientError/serverError -> raiseError(code, reason)

_For earlier versions, see [releases on GitHub](https://github.com/fhessel/esp32_https_server/releases)._
