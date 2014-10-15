#!/usr/bin/python
import argparse

# Import the presentation builder source code
import src

if __name__ == '__main__':

  # Create the argument parser
  parser = argparse.ArgumentParser(description='A wiki presnetation builder')
  parser.add_argument('input', type=str, help='Input file name')
  args = parser.parse_args()

  # Build the presentation
  builder = src.base.PresentationBuilder(args.input)
  builder.write()
