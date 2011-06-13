#!/usr/bin/env python

import os, string, re
from optparse import OptionParser

global_ignores = ['contrib', '.svn']
global_options = {}

def fixupHeader():
  for dirpath, dirnames, filenames in os.walk(os.getcwd() + "/../../"):

    # Don't traverse into ignored directories
    for ignore in global_ignores:
      if ignore in dirnames:
        dirnames.remove(ignore)

    #print dirpath
    #print dirnames
    for file in filenames:
      suffix = os.path.splitext(file)
      if suffix[-1] == '.py' and dirpath.find(r"tests/") != -1:
        checkAndUpdate(dirpath + '/' + file)
         

def checkAndUpdate(filename):
  found_try = False
  f = open(filename)
  new_f = open(filename + "~", "w")

  line = f.readline()
  strip_len = 0
  while (line != ""):
    if line.find("try:") != -1:
      found_try = True

      # Find first indented non-blank line and grab whitespace in front to know trim lenght
      not_found = True
      while not_found:
        line = f.readline()
        match = re.match("^\s*$", line)
        if match == None:
          match = re.match("^\s+", line)
          strip_len = len(match.group(0))
          not_found = False  

    if found_try and line.find("except:") == -1 and line.find("pass") == -1:
      if len(line) > strip_len:
        new_f.write(line[strip_len:])
      else:
        new_f.write(line)

    line = f.readline()

  f.close()
  new_f.close()

  os.rename(filename + "~", filename)

if __name__ == '__main__':
  parser = OptionParser()
  parser.add_option("-u", "--update", action="store_true", dest="update", default=False)
  parser.add_option("-v", "--verbose", action="store_true", dest="verbose", default=False)
  (global_options, args) = parser.parse_args()
  fixupHeader()
