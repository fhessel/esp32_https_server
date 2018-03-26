# ESP32 HTTPS Server

**Disclaimer**: This code is work-in-progress! It may contain several bugs, but it should be roughly working

This repository contains an HTTPS server implementation that can be used with the [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32).

The main goals for the server are:
- Ability to handle multiple clients in parallel (works)
- Possibility to be executed in a task without interrupting the main program (works)
- Ability to handle `Connection: keep-alive` (works) and WebSockets (tbd) to reduce the SSL-overhead for streaming data operations
- Abstraction of handling the HTTP(S) protocol (tbd)

For the future, the project structure will be converted to serve as a regular Arduino Library. For the moment, you may just
copy the https folder into your project, it is self-contained and needs only the WiFi-Library.

## Setup instructions

Roughly:

- Import project into [Sloeber](http://eclipse.baeyens.it/)
- Copy data/wifi/wifi.example.h to /data/wifi/wifi.h and change SSID and PSK
- Run tools/cert/create_cert.sh to create header files with cert data.

## Usage

Until there is an explicit documentation, the file https_server.cpp contains a script that shows how the server can be
used and be integrated in a project including comments on the various functions.

After compiling and flashing the script to your ESP32 module, you should be able to access the following resources on
the esp via HTTPS:

- your-ip/ - Welcome page showing the uptime and some parameterized SVG-images
- your-ip/favicon.ico - Favicon that is stored in data/favicon.h
- /images/awesome.svg?color=de58fe - SVG-Image, using the optional parameter as background color
- /echo - Will return request body for POST and PUT requests
- /param/[a]/[b] - Will show the parsed parameters a and b
- Any OPTIONS request - Will return some CORS-headers
- Everything else - JSON error object