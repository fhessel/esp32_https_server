#!/bin/bash

# This script verifies that every example in the examples directory can be built

# Check that PlatformIO is on the path
if [[ "$(which pio)" == "" ]]; then
  echo "::error::PlatformIO executable (pio) not found on PATH. Stop."
  echo "::error::PATH=$PATH"
  exit 1
fi

# Parse parameters
BOARD="$1"
if [[ "$BOARD" == "" ]]; then
  echo "::error::No board specified. Stop."
  exit 1
fi
EXAMPLENAME="$2"
if [[ "$EXAMPLENAME" == "" ]]; then
  echo "::error::No example specified. Stop."
  exit 1
fi

# In general, we want the script to fail if something unexpected happens.
# This flag gets only revoked for the actual build process.
set -e

# Find the script and repository location based on the current script location
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
REPODIR=$(cd "$(dirname $SCRIPTDIR/../../../..)" && pwd)

# Some other directory definitions
TMPDIR="$REPODIR/tmp"
CIDIR="$REPODIR/extras/ci"
EXAMPLEDIR="$REPODIR/examples"

# Re-create build directory
if [ -d "$TMPDIR" ]; then
  rm -r "$TMPDIR"
fi
mkdir -p "$TMPDIR"

# Check that an .ino file exists
if [ ! -d "$EXAMPLEDIR/$EXAMPLENAME" ]; then
  echo "::error::Example directory does not exist: $EXAMPLENAME"
  exit 1
fi
if [ ! -f "$EXAMPLEDIR/$EXAMPLENAME/$EXAMPLENAME.ino" ]; then
  echo "::error::Example sketch does not exist: $EXAMPLENAME.ino"
  exit 1
fi

# We take the .ino file, rename it as main.cpp and add an Arduino.h include at the top
PROJECTDIR="$TMPDIR/$EXAMPLENAME-$BOARD"
MAINCPP="$PROJECTDIR/src/main.cpp"
INOFILE="$PROJECTDIR/src/$EXAMPLENAME.ino"

# (re-)create the project directory under tmp/
if [ -d "$PROJECTDIR" ] && [ "$PROJECTDIR" != "" ]; then
  rm -r "$PROJECTDIR"
fi

# Create the lib folder to link the current version of the library
mkdir -p "$PROJECTDIR/lib"
# Copy the project folder template from ci/templates/example-project
cp -r "$CIDIR/templates/example-project"/* "$PROJECTDIR/"
# Copy the source files
cp -r "$EXAMPLEDIR/$EXAMPLENAME/." "$PROJECTDIR/src"
# Create the library link
ln -s "$REPODIR" "$PROJECTDIR/lib/esp32_https_server"
# Convert .ino to main.cpp
echo "#include <Arduino.h>" > "$MAINCPP"
cat "$INOFILE" >> "$MAINCPP"
rm "$INOFILE"

# If the example has dependencies, rewrite platformio.ini
if [[ -f "$EXAMPLEDIR/$EXAMPLENAME/.ci_lib_deps" ]]; then
  LIB_DEPS=$(head -n1 "$EXAMPLEDIR/$EXAMPLENAME/.ci_lib_deps")
  sed "s#\\#lib_deps#lib_deps = $LIB_DEPS#" "$PROJECTDIR/platformio.ini" > "$PROJECTDIR/platformio.ini.tmp"
  mv "$PROJECTDIR/platformio.ini.tmp" "$PROJECTDIR/platformio.ini"
fi

# Try building the application (+e as we want to test every example and get a
# summary on what is working)
set +e
pio --no-ansi run -d "$PROJECTDIR" -e "$BOARD" 2>&1 | \
  "$CIDIR/scripts/pio-to-gh-log.py" \
    "src/main.cpp:examples/$EXAMPLENAME/$EXAMPLENAME.ino:-1" \
    "lib/esp32_https_server/:src/" \
    "$REPODIR/:"
RC=${PIPESTATUS[0]}
if [[ "$RC" != "0" ]]; then
  echo "::error::pio returned with RC=$RC"
fi
exit $RC
