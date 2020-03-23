#!/bin/bash

# Find the script and repository location based on the current script location
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
REPODIR=$(cd "$(dirname $SCRIPTDIR/../../../..)" && pwd)

TOTAL_TESTS=0
FAILED_TESTS=0
OK_TESTS=0

EXAMPLES_SUCCESS=()
EXAMPLES_FAILURE=()
for BOARD in wrover wroom; do
  for EXAMPLENAME in $(ls "$REPODIR/examples"); do
    if [ -d "$REPODIR/examples/$EXAMPLENAME" ] && [ -f "$REPODIR/examples/$EXAMPLENAME/$EXAMPLENAME.ino" ]; then
      $SCRIPTDIR/build-example.sh "$BOARD" "$EXAMPLENAME"
      RC=$?
      TOTAL_TESTS=$[ TOTAL_TESTS + 1 ]
      if [[ "$RC" == "0" ]]; then
        OK_TESTS=$[ OK_TESTS + 1 ]
        EXAMPLES_SUCCESS+=("$EXAMPLENAME ($BOARD)")
      else
        FAILED_TESTS=$[ FAILED_TESTS + 1 ]
        EXAMPLES_FAILURE+=("$EXAMPLENAME ($BOARD)")
      fi
    fi
  done # example loop
done # board loop

echo "Summary: $OK_TESTS/$TOTAL_TESTS succeeded"
echo "-----------------------------------------"
for exmpl in "${EXAMPLES_SUCCESS[@]}"; do
  printf "  \u2714 $exmpl\n"
done
for exmpl in "${EXAMPLES_FAILURE[@]}"; do
  printf "  \u274c $exmpl\n"
done
if [[ "$FAILED_TESTS" != "0" ]]; then
  echo "$FAILED_TESTS Tests failed."
  exit 1
else
  echo "Success."
fi
