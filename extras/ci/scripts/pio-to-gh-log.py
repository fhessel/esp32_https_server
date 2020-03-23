#!/usr/bin/env python
import sys
import re

# Simple Python script that takes PlatformIO's compiler errors and maps them to
# output that can be understood by the Actions runner.

re_err = re.compile(r"^([^:]+):([0-9]+):([0-9]+): error: (.*)$")

for line in sys.stdin:
  print(line, end="")
  m = re_err.match(line.strip())
  if m is not None:
    print("::error file={name},line={line},col={col}::{message}".format(
      name=m.group(1), line=m.group(2), col=m.group(3), message=m.group(4)
    ))
