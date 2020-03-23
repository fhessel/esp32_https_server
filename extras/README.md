# Extras

This folder contains files that are not directly part of the library.

## CI – Continuous Integration

The [ci](ci) folder contains scripts and data used for automated testing.

## Documentation

The [docs](docs/) folder contains documentation about the internal structure
of the library.

## Legacy folder

Before the repository has been converted to follow the Arduino library
structure, the main documentation consisted of a large example sketch.
For reference, this sketch is archieved here.

## create_cert.sh

The script will create a CA and a server certificate that can be used to
run the example sketches. It requires OpenSSL and the xxd tool to convert
the DER-certificate data to C header files.

The certificate will not be trusted by any client, so you need to add a
security exception in your browser or use eg. the `--insecure` flag when
using tools like curl to test the server.

You should **not use this CA and certificates for production**. Make sure
that you know what you're doing.

The header files `cert.h` and `private_key.h` have been copied in every
example in the examples folder if the script terminated successfully, so you
don't need to care about any imports if you open the examples in the Arduino IDE
afterwards. The files should look like this:

```C++

/* cert.h */

unsigned char example_crt_DER[] = {
  0x30, 0x82, 0x02, 0x19, 0x30, 0x82, 0x01, 0x82, 0x02, 0x01,
  // ...
};
unsigned int example_crt_DER_len = 541;

/* private_key.h */

unsigned char example_key_DER[] = {
  0x30, 0x82, 0x02, 0x5c, 0x02, 0x01, 0x00, 0x02, 0x81, 0x81, 0x00, 0xe1,
  // ...
};
unsigned int example_key_DER_len = 608;

```
