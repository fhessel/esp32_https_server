# ESP32 HTTPS Server

**Disclaimer**: This code is work-in-progress and contains many non-existing features and bugs which are not documented.

This repository contains an HTTPS server implementation that can be used with the (ESP32 Arduino Core)[https://github.com/espressif/arduino-esp32].

The main goals for the server are:
- Ability to handle multiple clients in parallel
- Possibility to be executed in a task without interrupting the main program
- Ability to handle `Connection: keep-alive` and WebSockets to reduce the SSL-overhead for streaming data operations
- Abstraction of handling the HTTP(S) protocol

## Setup instructions

Roughly:

- Import project into [Sloeber](http://eclipse.baeyens.it/)
- Copy data/wifi/wifi.example.h to /data/wifi/wifi.h and change SSID and PSK
- Run tools/cert/create_cert.sh to create header files with cert data.

## Usage

(todo)