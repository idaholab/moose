#!/usr/bin/env python

import os, sys, re
from optparse import OptionParser

sys.path.append("./common")
import ParseGetPot

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


#re_print_linear = re.compile(r"^(\s*)([^#]*) print_linear_residuals\b \s*(.*)", re.X)
#re_ls31_option = re.compile(r"^([^#]*) -snes_type \s+ -snes_ls \s*(.*)", re.X)
#re_ls31_value = re.compile(r"^([^#]* petsc_options_value .*? ) ls \s+ (\w+) \s*(.*)", re.X)
#re_ls33_option = re.compile(r"^([^#]*) -snes_linesearch_type \s*(.*)", re.X)
#re_ls33_value = re.compile(r"^([^#]* petsc_options_value .*? ) \b(cubic | quadratic | basic | bt)\b \s*(.*)", re.X)

#re2 = re.compile(r"(.*)^([^#]*? petsc_options \s*=\s* (?: '\s*' | \"\s*\" | \s* ) )$", re.M | re.X | re.S)

def checkAndUpdate(filename):
  # Use the parser to find a specific parameter
  try:
    data = ParseGetPot.readInputFile(filename)
  except:        # ParseGetPot class
    print "*********************************************************************\nFailed to Parse " + filename + "\n"
    return

  if 'Executioner' in data.children:
    new_data = data.children['Executioner']
    if 'print_linear_residuals' in new_data.params:
      print filename
      update(filename)

  if 'Output' in data.children:
    new_data = data.children['Output']
    if 'print_linear_residuals' in new_data.params:
      print filename
      update(filename)


def update(filename):
  f = open(filename)
  lines = f.readlines()
  f.close()

  re_linear_res = re.compile(r"(\s*)([^#]*) print_linear_residuals.*\n", re.X)
  re_output = re.compile(r"([^#]*? \s* \[Output\].*)$(.*)$", re.X | re.M)   # .*$(.*)?$"

  replacement_made = False
#  line_search_added = False
  for i, line in enumerate(lines):

    # Remove print_linear_residuals
    (lines[i], num_replacements) = re_linear_res.subn("", lines[i])
    if num_replacements:
      replacement_made = True

    # Add linear_residuals
    m = re_output.search(lines[i])
    if m != None:
      (lines[i], num_replacements) = re_output.subn("\g<1>\n  linear_residuals = true", lines[i])
      replacement_made = True


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
#      lines[i] = re2.sub("\g<1>", lines[i])

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
