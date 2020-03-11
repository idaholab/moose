#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys, re, code

new_program = ''
f = open('large_sub_gen.py')
for line in f.readlines():
    match = re.search(r'(.*)cubit.cmd\((.*)\)(.*)', line, re.S)
    if (match != None):
        new_program += match.group(1) + 'print ' + match.group(2) + match.group(3)
        new_program += match.group(1) + 'cubit.silent_cmd(' + match.group(2) + ')' + match.group(3)
    else:
        new_program += line

ii = code.InteractiveInterpreter()
ii.runcode(code.compile_command(new_program, '<string>', 'exec'))

# TODO: Figure out how to capture the output from InteractiveInterpreter
#       so we don't see all the Cubit BS
#
#       I found a way to do this using the context manager class
#       and __future__ module but this doesn't exist in Python2.5

# For now the easiest thing to do is to pipe this command through grep
# All of the commands that I write begin with a lower case letter
# so the following RegEx should work

#  | grep "^[a-z]"
#  or
#  | grep "[[:lower:]]"
