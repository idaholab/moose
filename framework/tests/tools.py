import os
from subprocess import *

from TestHarness import TestHarness
import CSVDiffer

# make the TestHarness class a global of the tools.py module.
# We do this because all the legacy testing scripts use tools.TestHarness to
# start the test run, but the TestHarness class got so big it was refactored to
# another file
global TestHarness

def executeCommand(command):
  print 'Executing: ' + command

  p = Popen([command],stdout=PIPE,stderr=STDOUT, close_fds=True, shell=True)
  return p.communicate()[0]


def delOldOutFiles(test_dir, out_files):
  for file in out_files:
    try:
      os.remove(os.path.join(test_dir,file))
    except:
      pass

def executeApp(test_dir, input_file, min_dofs=0, parallel=0):
  saved_cwd = os.getcwd()
  os.chdir(test_dir)
  command = TestHarness.exec_name + ' -i ' + input_file
  if (parallel):
    command = 'mpiexec -np ' + str(parallel) + ' ' + command  
  if (min_dofs):
    command = 'time ' + command + ' --dofs ' + str(min_dofs)
  stdout = executeCommand(command)
  print stdout
  os.chdir(saved_cwd)

def executeExodiff(test_dir, out_files):
  for file in out_files:
    command = 'exodiff -F 1e-11 -use_old_floor -t 5.5E-6 ' + os.path.join(test_dir,file) + ' ' + os.path.join(test_dir,'gold',file)
    print command
    stdout = executeCommand(command)
    print stdout
    if stdout.find('different') != -1 or stdout.find('ERROR') != -1 or stdout.find('command not found') != -1:
      assert False

def diffCSV(test_dir, out_files):
  differ = CSVDiffer.CSVDiffer(test_dir, out_files)
  msgs = differ.diff()
  if msgs != '':
    print str( msgs.count('\n') ) + ' ERRORS:'
    print msgs
    assert False

def executeAppAndDiff(test_file, input_file, out_files, min_dofs=0, parallel=0):
  test_dir = os.path.dirname(test_file)
  delOldOutFiles(test_dir, out_files)
  executeApp(test_dir, input_file, min_dofs, parallel)
  if (min_dofs == 0 and parallel == 0):
    executeExodiff(test_dir, out_files)

def executeAppAndDiffCSV(test_file, input_file, out_files, abs_zero=1e-11, relative_error=5.5e-6):
  test_dir = os.path.dirname(test_file)
  delOldOutFiles(test_dir, out_files)
  executeApp(test_dir, input_file)
  diffCSV(test_dir, out_files)
