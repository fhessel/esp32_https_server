# Testing and Continuous Integration

This folder contains the scripts and data that is used to run the tests that are specified in the GitHub Action workflows.

The structure of this folder is as follows:

- **apps** contains applications that are used during CI testing
- **scripts** contains scripts and tools that are part of the workflows
- **templates** contains project skeletons etc. used for testing
- **tests** contains test suites written in Python which run against real hardware

## Run Tests Locally

You can (and should) run tests on your local machine before submitting an PR.

System Requirements

- A recent, Linux-based OS (it might work on macOS, too)
- Python 3 installed (`sudo apt-get install python3` on Debian or Ubuntu)
- Platform IO on your `PATH` (`source ~/.platformio/penv/bin/activate` if you're using an IDE like VSCode)
- For the hardware checks: An ESP32 development board connected to your machine and a WiFi access point shared between you and the ESP32

### Build Examples

You can use `extras/ci/scripts/build-example.sh <"wroom"|"wrover"> <example-name>` to build a specific example.
The main purpose of this is to check that everything still compiles as intended.
The script will only try to build the example, but it will not flash it.
The project folder is created in `tmp/<example-name>-<board-name>` in the root of the repositry.
If you want, you can run `pio run -t upload -e <"wroom"|"wrover">` in these project directories to upload the example to a board.

To build all examples (and to check that everything works), there is a script for manual testing: `extras/ci/scripts/build-example.sh`

### Run Tests on Hardware

(tbd)
