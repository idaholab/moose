#!/usr/bin/env python

# This script checks and can optionally update MOOSE source files.
# You should always run this script without the "-u" option
# first to make sure there is a clean dry run of the files that should
# be updated

import os, string
from optparse import OptionParser

global_ignores = ['contrib', '.svn']

copyright_header = \
"""/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

"""

global_options = {}

def fixupHeader():
  for dirpath, dirnames, filenames in os.walk(os.getcwd() + "/../"):

    # Don't traverse into ignored directories
    for ignore in global_ignores:
      if ignore in dirnames:
        dirnames.remove(ignore)

    #print dirpath
    #print dirnames
    for file in filenames:
      suffix = os.path.splitext(file)
      if suffix[-1] == '.C' or suffix[-1] == '.h':
        checkAndUpdate(dirpath + '/' + file)


def checkAndUpdate(filename):
  f = open(filename)
  text = f.read()
  f.close()

  # Check (exact match only)
  if (string.find(text, copyright_header) == -1):
    # print the first 10 lines or so of the file
    if global_options.update == False: # Report only
      print filename + ' does not contain an up to date header'
      if global_options.verbose == True:
        print '>'*40, '\n', '\n'.join((text.split('\n', 10))[:10]), '\n'*5
    else:
      # Update
      f = open(filename + '~tmp', 'w')
      f.write(copyright_header)
      f.write(text)
      f.close()
      os.rename(filename + '~tmp', filename)

if __name__ == '__main__':
  parser = OptionParser()
  parser.add_option("-u", "--update", action="store_true", dest="update", default=False)
  parser.add_option("-v", "--verbose", action="store_true", dest="verbose", default=False)
  (global_options, args) = parser.parse_args()
  fixupHeader()


