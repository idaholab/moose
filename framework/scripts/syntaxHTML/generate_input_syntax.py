#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys, os

if len(sys.argv) < 2:
    print('Usage: generate_input_syntax.py app_path app_name')
    exit(1)

app_path = sys.argv[1]

app_name = sys.argv[2]

MOOSE_DIR = os.path.abspath(os.path.join(os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]), '../../..'))))

FRAMEWORK_DIR = os.path.abspath(os.path.join(MOOSE_DIR, 'framework'))
#### See if MOOSE_DIR is already in the environment instead
if "MOOSE_DIR" in os.environ:
    MOOSE_DIR = os.environ['MOOSE_DIR']
    FRAMEWORK_DIR = os.path.join(MOOSE_DIR, 'framework')
if "FRAMEWORK_DIR" in os.environ:
    FRAMEWORK_DIR = os.environ['FRAMEWORK_DIR']

sys.path.append(MOOSE_DIR + '/framework/scripts/syntaxHTML')
import genInputFileSyntaxHTML

# this will automatically copy the documentation to the base directory
# in a folder named syntax
genInputFileSyntaxHTML.generateHTML(app_name, app_path, sys.argv, FRAMEWORK_DIR)
