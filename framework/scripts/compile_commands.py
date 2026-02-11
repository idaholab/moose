#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

# This script is used by the `make compile_commands.json` build command.
# It reformats the raw dump of current directory, compiler command, compiler
# options, and list of source files into a well formed JSON file that adheres to
# the format specified in https://clang.llvm.org/docs/JSONCompilationDatabase.html

import sys, os
import json

lines = [line.rstrip() for line in sys.stdin]

commands = []
for i in range(0, len(lines), 4):
    directory = lines[i]
    compiler = lines[i + 1]
    flags = lines[i + 2]
    sources = lines[i + 3].split()

    for file in sources:
        commands.append(
            {
                "directory": directory,
                "command": f"{compiler} {flags} -c {file}",
                "file": file,
            }
        )

print(json.dumps(commands, indent=2))
