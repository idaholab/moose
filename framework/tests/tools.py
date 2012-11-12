import sys, os

subdirs = ['testers']
module_path = os.path.dirname(__file__)
for subdir in subdirs:
  sys.path.append(module_path + '/' + subdir)

from TestTimer import TestTimer
from TestHarness import TestHarness

# Testers
from RunApp import RunApp
from Exodiff import Exodiff
from CSVDiff import CSVDiff
from RunException import RunException
from CheckFiles import CheckFiles

# Basic flow of control:
# initialize() - parse command line options, etc
# findTests() - find test.py files and import them
# prepareTest() - delete old output files, etc.
# createCommand() - create the command line to run
# run() - run the command line (since Popen spawns another process, it would be easy to run tests in parallel)
# testOutputAndFinish() - examine the output of the test to see if it passed or failed

## Called by ./run_tests in an application directory
def runTests(argv, app_name, moose_dir):
  if '--store-timing' in argv:
    # Pass control to TestTimer class for Test Timing
    harness = TestTimer(argv, app_name, moose_dir)
  else:
    harness = TestHarness(argv, app_name, moose_dir)

  # Registration
  harness.registerTester(RunApp, 'RunApp')
  harness.registerTester(Exodiff, 'Exodiff')
  harness.registerTester(CSVDiff, 'CSVDiff')
  harness.registerTester(RunException, 'RunException')
  harness.registerTester(CheckFiles, 'CheckFiles')

  harness.findAndRunTests()
