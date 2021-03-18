#!/usr/bin/env python
# gcode2ngc.py

import sys


# First line
print "%"

f = file(sys.argv[1])
for line in f:

    # Change extruder axis name
    line = line.replace(" E", " A")

    # S -> P
    line = line.replace(" S", " P")

    # Comment M82 code
    line = line.replace("M82", ";M82")

    # Comment M84 code
    line = line.replace("M84", ";M84")

    print line.strip()

# Last line
print "%"