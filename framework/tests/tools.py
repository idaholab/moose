import os, sys, re
from subprocess import *
from socket import gethostname

from TestHarness import TestHarness, ExodiffException
from LeakDetector import LeakDetector
import CSVDiffer

# if pysqlite2 isn't installed this class won't work, we don't care
# unless it was actually meant to be used
try:
  from TestTimer import TestTimer
except:
  pass

# make the TestHarness class a global of the tools.py module.
# We do this because all the legacy testing scripts use tools.TestHarness to
# start the test run, but the TestHarness class got so big it was refactored to
# another file
global TestHarness

################################################################################
############# this function is used by the run_tests scripts ###################
################################################################################
def runTests(argv, app_name):
  if '--helios-only-after' in argv and gethostname() != 'helios':
    return runTests(argv[:argv.index('--helios-only-after')], app_name)

  if '--memcheck' in argv or '-h' in argv or '--help' in argv:
    harness = LeakDetector(argv, app_name)
  elif '--store-timing' in argv:
    harness = TestTimer(argv, app_name)
  else:
    harness = TestHarness(argv, app_name)
  harness.runTestsAndExit()

################################################################################
########## these functions are used by the individual .py file tests ###########
################################################################################
def executeCommand(command):
  print 'Executing: ' + command

  p = Popen([command],stdout=PIPE,stderr=STDOUT, close_fds=True, shell=True)
  output = p.communicate()[0]
  if (p.returncode != 0):
    output = 'ERROR: ' + output
  return output

def delOldOutFiles(test_dir, out_files):
  for file in out_files:
    try:
      os.remove(os.path.join(test_dir,file))
    except:
      pass

def executeApp(test_dir, input_file, min_dofs=0, parallel=0, n_threads=0, expect_error=''):
  saved_cwd = os.getcwd()
  os.chdir(test_dir)
  command = TestHarness.exec_name + ' -i ' + input_file
  if (parallel):
    command = 'mpiexec -n ' + str(parallel) + ' ' + command  
  if (min_dofs):
    try:
      # First make sure the dang thing runs
      stdout = executeCommand(command)
      checkForFail(stdout)
      # Something is wrong so capture the output
    except:
      print stdout
      raise

    # Now we can safely capture the timing
    command = 'time ' + command + ' --dofs ' + str(min_dofs)
    
  if (n_threads):
    command += ' --n_threads=' + str(n_threads)
  stdout = executeCommand(command)
  print stdout
  if expect_error:
    checkExpectError(stdout, expect_error)
  else:
    checkForFail(stdout)
  os.chdir(saved_cwd)

def checkExpectError(output, expect_error):
  if re.search(expect_error, output, re.IGNORECASE) == None:
    print "%" * 100, "\nExpect Error Pattern not found:\n", expect_error, "\n", "%" * 100, "\n"
    assert False
  else: 
    assert True

def checkForFail(output):
  if output.find('different') != -1 or output.find('ERROR') != -1 or output.find('command not found') != -1:
    assert False

def executeExodiff(test_dir, out_files, abs_zero, relative_error):
  for file in out_files:
    command = 'exodiff -F ' + str(abs_zero) + ' -use_old_floor -t ' + str(relative_error) + ' ' + os.path.join(test_dir,file) + ' ' + os.path.join(test_dir,'gold',file)
    print command
    stdout = executeCommand(command)
    print stdout
    checkForFail(stdout)
#    if stdout.find('different') != -1 or stdout.find('ERROR') != -1 or stdout.find('command not found') != -1:
#      assert False

def diffCSV(test_dir, out_files):
  differ = CSVDiffer.CSVDiffer(test_dir, out_files)
  msgs = differ.diff()
  if msgs != '':
    print str( msgs.count('\n') ) + ' ERRORS:'
    print msgs
    assert False

def executeAppAndDiff(test_file, input_file, out_files, min_dofs=0, parallel=0, n_threads=0, abs_zero=1e-11, relative_error=5.5e-6):
  test_dir = os.path.dirname(test_file)
  delOldOutFiles(test_dir, out_files)
  executeApp(test_dir, input_file, min_dofs, parallel, n_threads)
  if (min_dofs == 0): #and parallel == 0):
    try:
      executeExodiff(test_dir, out_files, abs_zero, relative_error)
    except AssertionError:
      # We need to catch the failed exodiff and throw a different exception so we know it wasn't a normal failure
      # What a hack!
      raise ExodiffException()

def executeAppAndDiffCSV(test_file, input_file, out_files, min_dofs=0, parallel=0, n_threads=0, abs_zero=1e-11, relative_error=5.5e-6):
  test_dir = os.path.dirname(test_file)
  delOldOutFiles(test_dir, out_files)
  executeApp(test_dir, input_file, min_dofs, parallel, n_threads)
  diffCSV(test_dir, out_files)

def executeAppExpectError(test_file, input_file, expect_error=''):
  test_dir = os.path.dirname(test_file)
  executeApp(test_dir, input_file, 0, 0, 0, expect_error)
  
