#!/usr/bin/env python

import os, string
from optparse import OptionParser

global_ignores = ['fparser', '.svn']

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

copyright_header_old = \
"""/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

"""

def fixupHeader():
  for dirpath, dirnames, filenames in os.walk(os.getcwd() + "/../"):

    # Don't traverse into ignored directories
    for ignore in global_ignores:
      if ignore in dirnames:
        dirnames.remove(ignore)

    print dirpath
    print dirnames
    for file in filenames:
      suffix = os.path.splitext(file)
      if suffix[-1] == '.C' or suffix[-1] == '.h':
        checkAndUpdate(dirpath + '/' + file)
          

def checkAndUpdate(filename):
  f = open(filename)
  text = f.read()
  f.close()

  # Check (exact match only)
  if (string.find(text, copyright_header_old) != -1):
    print "Yes"
    text = text.replace(copyright_header_old, copyright_header)

    # Update
    f = open(filename + "~tmp", "w")
#    f.write(copyright_header)
    f.write(text)
    f.close()

    os.rename(filename + "~tmp", filename)
  
if __name__ == '__main__':
  fixupHeader()


