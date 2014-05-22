#!/usr/bin/env python
import os, sys

# Get the real path of create_dox.py
if(os.path.islink(sys.argv[0])):
  pathname = os.path.dirname(os.path.realpath(sys.argv[0]))
else:
  pathname = os.path.dirname(sys.argv[0])
  pathname = os.path.abspath(pathname)

sys.path.append(os.path.join(MOOSE_DIR, 'python'))
import path_tool
path_tool.activate_module('argparse')

import argparse

def writeDOX(args, writefile):
  args.file_object.write("\n/** @example " + writefile + "\n */")

def createDOX(args):
  file_dict = os.walk(args.application)
  for dir_name, dir_list, file_list in file_dict:
    for single_file in file_list:
      if os.path.splitext(dir_name + '/' + single_file)[1] == '.i':
        writeDOX(args, single_file)
  args.file_object.close()
  print 'Wrote to file:', args.application + '/' + args.application.split('/').pop() + '.dox'

def _verifyARGs(args):
  if os.path.exists(args.application):
    args.file_object = open(args.application + '/' + args.application.split('/').pop() + '.dox', 'w')
    return args
  else:
    print 'Path not found:', args.application
    sys.exit(1)

def _parseARGs(args=None):
  parser = argparse.ArgumentParser(description='Build dox file for every input file. (Deletes DOX file if present)')
  parser.add_argument('--application', required=True, help='Path to application')
  return _verifyARGs(parser.parse_args(args))

if __name__ == '__main__':
  createDOX(_parseARGs())
