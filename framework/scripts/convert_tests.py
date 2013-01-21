#!/usr/bin/python

import os, sys, re, inspect, types, subprocess
from optparse import OptionParser
sys.path.append(os.path.dirname(__file__) + '/../tests/')
from options import *

global_options = {}
global_args = []

# A listing of all of the options in the order we would like them to be in
master_order = ['type','input','exodiff','exodiff_options','abs_zero','rel_err','custom_cmp','use_old_floor','check_files','csvdiff','group','heavy','gold_dir','test_dir','should_crash','expect_err','expect_out','expect_assert','errors','max_parallel','min_parallel','max_threads','min_threads','max_time','platform','compiler','petsc_version','mesh_mode','method','cli_args','prereq','scale_refine','skip','deleted']

def findAndConvert():
  test_match = re.compile(r"(?:^|\b|[_-])[Tt]est")

  for dirpath, dirnames, filenames in os.walk(os.getcwd() + '/' + args[0]):
    for file in filenames:
      saved_cwd = os.getcwd()
      sys.path.append(os.path.abspath(dirpath))
      os.chdir(dirpath)

      if file[-2:] == 'py' and test_match.search(file): # Legacy file formatted test
        tests = parseLegacyTestFormat(file, test_match)

        f = open('tests', 'w')
        f.write("[Tests]")

        for test_name, test_opts in tests.items():
          f.write ("\n  [./" + test_name + "]\n")

          if 'exodiff' in test_opts:
            test_opts['type'] = 'Exodiff'
          elif 'csvdiff' in test_opts:
            test_opts['type'] = 'CSVDiff'
          elif 'expect_err' in test_opts:
            test_opts['type'] = 'RunException'
          elif 'expect_assert' in test_opts:
            test_opts['type'] = 'RunException'
          elif 'expect_out' in test_opts:
            test_opts['type'] = 'RunApp'
          else:
            test_opts['type'] = 'RunApp'

          keys = []

          for key in master_order:
            if key in test_opts:
              value = test_opts[key]
              if type(value) == list:
                value = "'" + ' '.join(value) + "'"
              elif type(value) == str:
                value = "'" + value + "'"
              else:
                value = str(value)
              f.write("    " + key + " = " + value + '\n')
          f.write("  [../]\n")
        f.write("[]\n")

        subprocess.call(['git', 'rm', file])
        print "git rm " + file

      os.chdir(saved_cwd)
      sys.path.pop()

def parseLegacyTestFormat(filename, test_match):
  # dynamically load the module
  module_name = filename[:-3]	# Always a python file (*.py)
  module = __import__(module_name)
  test_dir = os.path.dirname(module.__file__)
  tests = {}

  for test_name, test_opts in inspect.getmembers(module):
    if isinstance(test_opts, types.DictType) and test_match.search(test_name):
      tests[test_name] = test_opts

  return tests

if __name__ == '__main__':
  parser = OptionParser()
  (global_options, args) = parser.parse_args()
  if len(args) != 2:
    findAndConvert()
