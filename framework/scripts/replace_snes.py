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


#re_snes = re.compile(r"^(\s*)([^#]*) -ksp_monitor\b \s*(.*)", re.X)
re_option = re.compile(r"^(\s*)([^#]*) petsc_options_iname \s* = \s* '?(.*?)'? \s* $", re.X)
re_value  = re.compile(r"^(\s*)([^#]*) petsc_options_value \s* = \s* '?(.*?)'? \s* $", re.X)
#re_ls33_option = re.compile(r"^([^#]*) -snes_linesearch_type \s*(.*)", re.X)
#re_ls33_value = re.compile(r"^([^#]* petsc_options_value .*? ) \b(cubic | quadratic | basic | bt)\b \s*(.*)", re.X)

re2 = re.compile(r"(.*)^([^#]*? petsc_options_iname \s*=\s* (?: '\s*' | \"\s*\" | \s* ) )$", re.M | re.X | re.S)
re3 = re.compile(r"(.*)^([^#]*? petsc_options_value \s*=\s* (?: '\s*' | \"\s*\" | \s* ) )$", re.M | re.X | re.S)

def checkAndUpdate(filename):
  f = open(filename)
  lines = f.readlines()
  f.close()

  replacement_made = False
  line_search_added = False

  option_idx = []
  option_lens = []

  try:
    for i, line in enumerate(lines):

      # Replace PETSc solver type
  #    (lines[i], num_replacements) = re_snes.subn("\n\g<1>print_linear_residuals = true\n\g<1>\g<2>\g<3>", lines[i])
  #    if num_replacements:
  #      replacement_made = True
  #      print filename

      # Replace PETSC pc options
      m1 = re_option.search(lines[i])
      if m1:
        print filename
        options = re.split('\s+', m1.group(3).strip())

        replace_string = ''
        for j, option in enumerate(options):
          option_lens.append(len(option))
          if option == '-pc_type' or option == '-pc_hypre_type' or option == '-sub_pc_type':
            option_idx.append(j)
          else:
            replace_string = replace_string + option + ' '
        replace_string = replace_string.strip()

        lines[i] = m1.group(1) + m1.group(2) + "petsc_options_iname = '" +  replace_string + "'\n"
        replacement_made = True

      if len(option_idx):
        m2 = re_value.search(line)
        if m2:
          values = re.split('\s+', m2.group(3).strip())

          replace_string2 = ''
          precond = 'UNKNOWN'
          for j, value in enumerate(values):
            if j in option_idx:
              if value in ('boomeramg, ilu, lu, asm'):
                precond = value
            else:
              replace_string2 = replace_string2 + value.ljust(option_lens[j]) + ' '
          replace_string2 = replace_string2.strip()

          # Abort early if we can't figure out the petsc options lines
          if precond == 'UNKNOWN':
            return
          lines[i] = m2.group(1) + m2.group(2) + "petsc_options_value = '" +  replace_string2 + "'\n" + m2.group(1) + m2.group(2) + "preconditioner = '" + precond + "'\n"



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
        lines[i] = re3.sub("\g<1>", lines[i])
  except:
    print "Error processing " + filename
    return

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
