#!/usr/bin/env python
import sys, os, inspect

# Set the current working directory to the directory where this script is located
os.chdir(os.path.abspath(os.path.dirname(sys.argv[0])))

#### Set the name of the application here and moose directory relative to the application
app_name = 'stork'

MOOSE_DIR = os.path.abspath(os.path.join('..', 'moose'))
#### See if a submodule is available
if os.path.exists(os.path.abspath(os.path.join('moose', 'framework', 'Makefile'))):
  MOOSE_DIR = os.path.abspath('moose')
#### See if MOOSE_DIR is already in the environment instead
if os.environ.has_key("MOOSE_DIR"):
  MOOSE_DIR = os.environ['MOOSE_DIR']

sys.path.append(os.path.join(MOOSE_DIR, 'python'))
import path_tool
path_tool.activate_module('TestHarness')

from TestHarness import TestHarness
# Run the tests!
TestHarness.buildAndRun(sys.argv, app_name, MOOSE_DIR)
