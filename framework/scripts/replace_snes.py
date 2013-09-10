#!/usr/bin/env python

# This script checks and can optionally update MOOSE source files.
# You should always run this script without the "-u" option
# first to make sure there is a clean dry run of the files that should
# be updated

import os, sys, re
from optparse import OptionParser

global_ignores = ['archive', 'contrib', '.svn', '.git']

global_options = {}

def fixupHeader():
  for dirpath, dirnames, filenames in os.walk(os.getcwd() + "/../../../"):

    # Don't traverse into ignored directories
    for ignore in global_ignores:
      if ignore in dirnames:
        dirnames.remove(ignore)

    #print dirpath
    #print dirnames
    for file in filenames:
      suffix = os.path.splitext(file)
      if suffix[-1] == '.i':
        checkAndUpdate(dirpath + '/' + file)


re_snes = re.compile(r"^(\s*)([^#]*) -snes_mf\b \s*(.*)", re.X)
#re_ls31_option = re.compile(r"^([^#]*) -snes_type \s+ -snes_ls \s*(.*)", re.X)
#re_ls31_value = re.compile(r"^([^#]* petsc_options_value .*? ) ls \s+ (\w+) \s*(.*)", re.X)
#re_ls33_option = re.compile(r"^([^#]*) -snes_linesearch_type \s*(.*)", re.X)
#re_ls33_value = re.compile(r"^([^#]* petsc_options_value .*? ) \b(cubic | quadratic | basic | bt)\b \s*(.*)", re.X)

re2 = re.compile(r"(.*)^([^#]*? petsc_options \s*=\s* (?: '\s*' | \"\s*\" | \s* ) )$", re.M | re.X | re.S)
re_pre = re.compile(r"^[^#]* type \s* = \s* PBP", re.X)

def checkAndUpdate(filename):
  f = open(filename)
  lines = f.readlines()
  f.close()

  solve_type = 'PJFNK'
  for i, line in enumerate(lines):
    if re_pre.search(line) != None:
      solve_type = 'JFNK'

  replacement_made = False
  line_search_added = False
  for i, line in enumerate(lines):

    # Replace PETSc solver type
    (lines[i], num_replacements) = re_snes.subn("\n\g<1>solve_type = " + solve_type + "\n\g<1>\g<2>\g<3>", lines[i])
    if num_replacements:
      replacement_made = True
      print filename + " switching to " + solve_type

    # Replace PETSC3.1 linesearch options
#    (lines[i], num_replacements) = re_ls31_option.subn("\g<1>\g<2>", lines[i])
#    if num_replacements:
#      replacement_made = True
#
#    m = re_ls31_value.search(lines[i])
#    if m:
#      replacement_made = True
#      line_search_added = True
#      line_search_type = m.group(2)
#      if line_search_type == "basic":
#        line_search_type = "none"
#
#      (lines[i], num_replacements) = re_ls31_value.subn("\g<1>\g<3>\n\n  line_search = '" + line_search_type + "'\n", lines[i])
#
#    # Replace PETSC3.3 linesearch options
#    (lines[i], num_replacements) = re_ls33_option.subn("\g<1>\g<2>", lines[i])
#    if num_replacements:
#      replacement_made = True
#
#    m = re_ls33_value.search(lines[i])
#    if m:
#      replacement_made = True
#      line_search_type = m.group(2)
#      if line_search_type == "basic":
#        line_search_type = "none"
#
#      if line_search_added:
#        (lines[i], num_replacements) = re_ls33_value.subn("\g<1>\g<3>\n", lines[i])
#      else:
#        (lines[i], num_replacements) = re_ls33_value.subn("\g<1>\g<3>\n\n  line_search = '" + line_search_type + "'\n", lines[i])

    # See if we've left an empty set of options now that we've replaced the solver type, remove if necessary
    if replacement_made:
      lines[i] = re2.sub("\g<1>", lines[i])

    f = open(filename + '~tmp', 'w')
    f.write(''.join(lines))
    f.close()
    os.rename(filename + '~tmp', filename)

if __name__ == '__main__':
  parser = OptionParser()
  parser.add_option("-u", "--update", action="store_true", dest="update", default=False)
  parser.add_option("-v", "--verbose", action="store_true", dest="verbose", default=False)
  (global_options, args) = parser.parse_args()
  fixupHeader()
