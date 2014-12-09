#!/usr/bin/python
import os, sys
import argparse

# Should just add ~/projects/moose/python to PYTHONPATH
MOOSE_DIR = os.path.abspath(os.path.join('..','..'))
if os.environ.has_key("MOOSE_DIR"):
  MOOSE_DIR = os.environ['MOOSE_DIR']
sys.path.append(os.path.join(MOOSE_DIR, 'python'))
from FactorySystem import ParseGetPot

# Import the presentation builder source code
from PresentationBuilder import base

if __name__ == '__main__':

  # Create the argument parser
  parser = argparse.ArgumentParser(description='A wiki presnetation builder')
  parser.add_argument('input', type=str, help='Input file name')
  parser.add_argument('--format', '-f', type=str, default='remark', help='Select the presentation output format (remark | reveal)')
  args = parser.parse_args()

  # Build the presentation
  builder = base.PresentationBuilder(args.input, format=args.format)
  builder.write()
