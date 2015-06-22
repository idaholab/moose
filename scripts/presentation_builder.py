#!/usr/bin/python
import os, sys
import argparse

# Determine the MOOSE Directory
MOOSE_PYTHON_DIR = None
if os.environ.has_key('MOOSE_DIR'):
  MOOSE_PYTHON_DIR = os.path.join(os.environ['MOOSE_DIR'], 'python')
else:
  MOOSE_PYTHON_DIR = os.path.join(os.environ['HOME'], 'projects', 'moose', 'python')

# Add moose/python to path
if os.path.exists(MOOSE_PYTHON_DIR):
  sys.path.append(MOOSE_PYTHON_DIR)
else:
  raise Exception('Unable to locate moose/python directory, please set MOOSE_DIR environment variable')

# Load the required moose/python packages
from FactorySystem import ParseGetPot
from PresentationBuilder import base

if __name__ == '__main__':

  # Create the argument parser
  parser = argparse.ArgumentParser(description='A wiki presentation builder')
  parser.add_argument('input', type=str, help='Input file name')
  parser.add_argument('--format', '-f', type=str, default='remark', help='Select the presentation output format (remark | reveal)')
  args = parser.parse_args()

  # Build the presentation
  builder = base.PresentationBuilder(args.input, format=args.format)
  builder.write()
