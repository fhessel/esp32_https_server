#!/usr/bin/env python
import sys
import re

# Simple Python script that takes PlatformIO's compiler errors and maps them to
# output that can be understood by the Actions runner.

re_err = re.compile(r"^([^:]+):([0-9]+):([0-9]+): error: (.*)$")

# Parameters are strings of the form
# path_prefix:replacement_prefix:line_offset
# Where all paths starting with path_prefix will be replced with replacement_prefix,
# and if such a replacement takes place, the line number will be shifted by line_offset.
# That allows taking care for inserted code like the #include <Arduino.h>
mappings = []
for arg in sys.argv[1:]:
  parts = arg.split(":", 2)
  mappings.append((*parts[0:2], 0 if len(parts)==2 else int(parts[2])))

for line in sys.stdin:
  print(line, end="")
  m = re_err.match(line.strip())
  if m is not None:
    name = m.group(1)
    lineno = int(m.group(2))
    for mapping in mappings:
      if name.startswith(mapping[0]):
        name = mapping[1] + name[len(mapping[0]):]
        lineno += mapping[2]
    print("::error file={name},line={line},col={col}::{message}".format(
      name=name, line=lineno, col=m.group(3), message=m.group(4)
    ))
