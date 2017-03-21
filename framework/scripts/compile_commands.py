#!/usr/bin/env python
import sys, os
import json

lines = [line.rstrip() for line in sys.stdin]

commands = []
for file in lines[3].split() :
  commands.append({
    'directory' : lines[0],
    'command' : "%s %s -c %s" % (lines[1], lines[2], file),
    'file' : file
  })

print json.dumps(commands, indent=2)
