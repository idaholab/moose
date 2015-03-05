#!/usr/bin/python
import os, sys
import argparse

# Locate the moose/python directory
sys.path.append(os.path.join(os.path.abspath('..'), 'python'))

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
