#!/bin/bash

# This script verifies that every example in the examples directory can be built

# Check that PlatformIO is on the path
if [[ "$(which pio)" == "" ]]; then
  echo "PlatformIO executable (pio) not found on PATH. Stop."
  echo "PATH=$PATH"
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

# Count failed tests
FAILED_TESTS=0

# Lists for failed and successful tests for the summary
EXAMPLES_SUCCESS=()
EXAMPLES_FAILURE=()

# For each example
for EXAMPLE in "$EXAMPLEDIR"/*; do
  # Check that an .ino file exists
  EXAMPLENAME=$(basename $EXAMPLE)
  if [ -d "$EXAMPLE" ] && [ -f "$EXAMPLE/$EXAMPLENAME.ino" ]; then
    if [ "$#" -ne 1 ]; then
      # No arguments: Test all
      TEST_THIS_EXAMPLE=1
    else
      # Has arguments: Test only examples listed as arguments
      TEST_THIS_EXAMPLE=0
      for arg in "$@"; do
        [[ $arg == "$EXAMPLENAME" ]] && TEST_THIS_EXAMPLE=1 && break
      done
    fi
    # If the test should be executed
    if [[ "$TEST_THIS_EXAMPLE" == "1" ]]; then
      # We take the .ino file, rename it as main.cpp and add an Arduino.h include at the top
      MAINCPP="$TMPDIR/$EXAMPLENAME/src/main.cpp"
      INOFILE="$TMPDIR/$EXAMPLENAME/src/$EXAMPLENAME.ino"
      PROJECTDIR="$TMPDIR/$EXAMPLENAME"
      echo "Building $EXAMPLENAME"
      echo "------------------------------------------------------------"
      # (re-)create the project directory under tmp/
      if [ -d "$PROJECTDIR" ] && [ "$PROJECTDIR" != "" ]; then
        rmdir -r "$PROJECTDIR"
      fi
      # Create the lib folder to link the current version of the library
      mkdir -p "$PROJECTDIR/lib"
      # Copy the project folder template from ci/templates/example-project
      cp -r "$CIDIR/templates/example-project"/* "$PROJECTDIR/"
      # Copy the source files
      cp -r "$EXAMPLEDIR/$EXAMPLENAME/." "$PROJECTDIR/src"
      # Create the library link
      ln -s "$REPODIR" "$PROJECTDIR/lib/pio-esp32-ci-demo"
      # Convert .ino to main.cpp
      echo "#include <Arduino.h>" > "$MAINCPP"
      cat "$INOFILE" >> "$MAINCPP"
      rm "$INOFILE"
      # Try building the application (+e as we want to test every example and get a
      # summary on what is working)
      set +e
      pio run -d "$TMPDIR/$EXAMPLENAME"
      SUCCESS=$?
      set -e
      # Evaluate this example
      if [[ "$SUCCESS" != "0" ]]; then
        FAILED_TESTS=$[ FAILED_TESTS + 1 ]
        EXAMPLES_FAILURE+=("$EXAMPLENAME")
      else
        EXAMPLES_SUCCESS+=("$EXAMPLENAME")
      fi
    fi # TEST_THIS_EXAMPLE
  fi # example dir exists and contains .ino
done

# Summarize the results
echo "Summary"
echo "------------------------------------------------------------"
for exmpl in "${EXAMPLES_SUCCESS[@]}"; do
  printf " \u2714 $exmpl\n"
done

for exmpl in "${EXAMPLES_FAILURE[@]}"; do
  printf " \u274c $exmpl\n"
done

# Return the overall success
if [[ "$FAILED_TESTS" != "0" ]]; then
  echo "$FAILED_TESTS Tests failed."
  exit 1
else
  echo "Success."
fi
