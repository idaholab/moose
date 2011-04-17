#!/usr/bin/env python
import sys, os

# get the location of this script
app_path = os.path.abspath(os.path.dirname(sys.argv[0]))
# this script is actually in the scripts subdirectory, so go up a level
app_path += '/..'

# Set the name of the application here and moose directory relative to the application
app_name = 'moose_test'
MOOSE_DIR = app_path + '/../moose'
# See if MOOSE_DIR is already in the environment instead
if os.environ.has_key("MOOSE_DIR"):
  MOOSE_DIR = os.environ['MOOSE_DIR']

sys.path.append(MOOSE_DIR + '/scripts/syntaxHTML')
import genInputFileSyntaxHTML

# this will automatically copy the documentation to the base directory
# in a folder named syntax
genInputFileSyntaxHTML.generateHTML(app_name, app_path, sys.argv, MOOSE_DIR)
