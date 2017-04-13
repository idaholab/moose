#!/usr/bin/env python

# This script is used by the `make compile_commands.json` build command.
# It reformats the raw dump of current directory, compiler command, compiler
# options, and list of source files into a well formed JSON file that adheres to
# the format specified in https://clang.llvm.org/docs/JSONCompilationDatabase.html

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
