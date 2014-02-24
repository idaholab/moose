#!/usr/bin/env python
import sys, os

# get the location of this script
app_path = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]), ".."))

# Set the name of the application here and moose directory relative to the application
app_name = 'bison'

MOOSE_DIR = os.path.abspath(os.path.join(app_path, '..'))
FRAMEWORK_DIR = os.path.abspath(os.path.join(app_path, '..', 'framework'))
#### See if MOOSE_DIR is already in the environment instead
if os.environ.has_key("MOOSE_DIR"):
  MOOSE_DIR = os.environ['MOOSE_DIR']
  FRAMEWORK_DIR = os.path.join(MOOSE_DIR, 'framework')
if os.environ.has_key("FRAMEWORK_DIR"):
  FRAMEWORK_DIR = os.environ['FRAMEWORK_DIR']

sys.path.append(MOOSE_DIR + '/scripts/syntaxHTML')
import genInputFileSyntaxHTML

# this will automatically copy the documentation to the base directory
# in a folder named syntax
genInputFileSyntaxHTML.generateHTML(app_name, app_path, sys.argv, FRAMEWORK_DIR)
