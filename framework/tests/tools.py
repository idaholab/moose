from socket import gethostname
from TestTimer import TestTimer
from TestHarness import TestHarness

# Basic flow of control:
# initialize() - parse command line options, etc
# findTests() - find test.py files and import them
# prepareTest() - delete old output files, etc.
# createCommand() - create the command line to run
# run() - run the command line (since Popen spawns another process, it would be easy to run tests in parallel)
# testOutputAndFinish() - examine the output of the test to see if it passed or failed

## Called by ./run_tests in an application directory
def runTests(argv, app_name, moose_dir):
  host_name = gethostname()
  if host_name == 'service0' or host_name == 'service1':
    print 'Testing not supported on Icestorm head node'
    sys.exit(0)
  if '--store-timing' in argv:
    # Pass control to TestTimer class for Test Timing
    harness = TestTimer(argv, app_name, moose_dir)
    harness.findAndRunTests()
  else:
    harness = TestHarness(argv, app_name, moose_dir)
    harness.findAndRunTests()
