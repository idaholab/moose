import os, sys, re, inspect, types
from socket import gethostname
from optparse import OptionParser
from timeit import default_timer as clock

from options import *
from util import *
from RunParallel import RunParallel
from CSVDiffer import CSVDiffer


# Basic flow of control:
# initialize() - parse command line options, etc
# findTests() - find test.py files and import them
# prepareTest() - delete old output files, etc.
# createCommand() - create the command line to run
# run() - run the command line (since Popen spawns another process, it would be easy to run tests in parallel)
# testOutputAndFinish() - examine the output of the test to see if it passed or failed

## Called by ./run_tests in an application directory
def runTests(argv, app_name):
  host_name = gethostname()
  if host_name == 'service0' or host_name == 'service1':
    print 'Testing not supported on Icestorm head node'
    sys.exit(0)

  harness = TestHarness(argv, app_name)
  harness.findAndRunTests()


class TestHarness:
  
  def __init__(self, argv, app_name):
    self.initialize(argv, app_name)
    self.test_table = []

    self.num_passed = 0
    self.num_failed = 0
    self.num_skipped = 0

  def findAndRunTests(self):
    self.start_time = clock()
    for dirpath, dirnames, filenames in os.walk(os.getcwd()):
      if (self.test_match.search(dirpath)):
        for file in filenames:
          # See if there were other arguments (test names) passed on the command line
          if (file[-2:] == 'py' and  # TODO make it so if a leftover appears in any test_name it will run
              self.test_match.search(file) and
              (len(self.tests) == 0 or len([item for item in self.tests if re.match(item, file)]) == 1)):

            module_name = file[:-3]
            saved_cwd = os.getcwd()  
            sys.path.append(os.path.abspath(dirpath))
            os.chdir(dirpath)

            # dynamically load the module
            module = __import__(module_name)
            test_dir = os.path.dirname(module.__file__)

            # look for dicts that match the test regex
            for test_name, test_opts in inspect.getmembers(module):
              if isinstance(test_opts, types.DictType) and self.test_match.search(test_name):
                # insert default values where none provided
                testname = module_name + '.' + test_name
                test = DEFAULTS.copy()
                test.update(test_opts)
                test.update( { TEST_NAME : testname, TEST_DIR : test_dir } )

                if self.checkIfRunTest(test):
                  self.prepareTest(test)
                  execute = self.createCommand(test)

                  # This method spawns another process and allows this loop to continue looking for tests
                  # RunParallel will call self.testOutputAndFinish when the test has completed running
                  # This method will block when the maximum allowed parallel processes are running
                  self.runner.run(test, execute)
                                
            os.chdir(saved_cwd)
            sys.path.pop()

    self.cleanupAndExit()

  # Create the command line string to run
  def createCommand(self, test):
    return self.executable + ' -i ' + test[INPUT]

  ## Delete old output files
  def prepareTest(self, test):
    for file in (test[CSVDIFF] + test[EXODIFF]):
      try:
        os.remove(os.path.join(test[TEST_DIR], file))
      except:
        pass

  # If the test is not to be run for any reason, print skipped as the result and return False,
  # otherwise return True
  def checkIfRunTest(self, test):
    if test[SKIP]:
      self.handleTestResult(test, '', 'skipped')
      return False
    return True

  ## Finish the test by inspecting the raw output 
  def testOutputAndFinish(self, test, retcode, output):
    reason = ''
    
    # Check the general error message and program crash possibilities
    if len( filter( lambda x: x in output, test[ERRORS] ) ) > 0:
      reason = 'ERRMSG'
    elif test[EXPECT_ERR] != None and test[EXPECT_ERR] not in output:
      reason = 'NO EXPECTED ERR'
    elif retcode == RunParallel.TIMEOUT:
      reason = 'TIMEOUT'
    elif retcode != 0 and not test[SHOULD_CRASH]:
      reason = 'CRASH'
    elif retcode > 0 and test[SHOULD_CRASH]:
      reason = 'NO CRASH'
    else:  # Now test more involved things like CSV and EXODIFF
      for file in test[EXODIFF]:
        command = 'exodiff -m -F ' + str(test[EXO_OPTIONS][ABS_ZERO]) + ' -use_old_floor -t ' + str(test[EXO_OPTIONS][REL_ERR]) \
                  + ' ' + os.path.join(test[TEST_DIR], file) + ' ' + os.path.join(test[TEST_DIR], test[GOLD_DIR], file)
        exo_output = runCommand(command)
        output += 'Running exodiff: ' + command + '\n' + exo_output

        if 'different' in exo_output or 'ERROR' in exo_output:
          reason = 'EXODIFF'
          break;

      # if still no errors, diff CSVs
      if reason == '' and len(test[CSVDIFF]) > 0:
        differ = CSVDiffer( test[TEST_DIR], test[CSVDIFF] )
        msg = differ.diff()
        output += 'Running CSVDiffer.py\n' + msg
        if msg != '':
          reason = 'CSVDIFF'
    
    result = 'OK' if reason == '' else 'FAILED (%s)' % reason
    self.handleTestResult(test, output, result)


  ## Update global variables and print output based on the test result
  # OK means it passed, skipped means skipped, anything else means it failed
  def handleTestResult(self, test, output, result):
    self.test_table.append( (test, output, result) )
    printResult( test[TEST_NAME], result, self.options)

    if result == 'OK':
      self.num_passed += 1
    elif result == 'skipped':
      self.num_skipped += 1
    else:
      self.num_failed += 1

  def cleanupAndExit(self):
    time = clock() - self.start_time
    print '-' * 80
    print 'Ran %d tests in %.1f seconds' % (self.num_passed+self.num_failed, time)

    summary = '<g>%d passed</g>' if self.num_passed else '<b>%d passed</b>'
    summary += ', <b>%d skipped</b>, '
    summary += '<r>%d FAILED</r>' if self.num_failed else '<b>%d failed</b>'
    print colorify( summary % (self.num_passed, self.num_skipped, self.num_failed), self.options, html=True )

    if self.num_failed == 0:
      sys.exit(0)
    else:
      sys.exit(1)

  def initialize(self, argv, app_name):
    self.parseCLArgs(argv)

    # Initialize the parallel runner with how many tests to run in parallel
    self.runner = RunParallel(self, self.options.jobs)
    
    ## Save executable-under-test name to self.executable
    method = ''
    if self.options.method:
      method = self.options.method
    elif os.environ.has_key('METHOD'):
      method = os.environ['METHOD']
    else:
      method = 'opt'
    self.executable = os.getcwd() + '/' + app_name + '-' + method

    # Emulate the standard Nose RegEx for consistency
    self.test_match = re.compile(r"(?:^|\b|[_-])[Tt]est")

  ## Parse command line options and assign them to self.options
  def parseCLArgs(self, argv):
    parser = OptionParser()
    parser.add_option('-v', '--verbose', action='store_true', dest='verbose', default=False, help='show more output [default=FALSE]')
    parser.add_option('--opt', action='store_const', dest='method', const='opt', help='test the app_name-opt binary')
    parser.add_option('--dbg', action='store_const', dest='method', const='dbg', help='test the app_name-dbg binary')
    parser.add_option('--dev', action='store_const', dest='method', const='dev', help='test the app_name-dev binary')
    parser.add_option('-j', '--jobs', action='store', type='int', dest='jobs', default=1, help='run test binaries in parallel')
    parser.add_option("-c", "--no-color", action="store_false", dest="colored", default=True, help="Do not show colored output")

    (self.options, self.tests) = parser.parse_args(argv[1:])
    self.checkCLArgs()

  ## Called after options are parsed from the command line
  # Exit if options don't make any sense, print warnings if they are merely weird
  def checkCLArgs(self):
    pass


# Notes:
# SHOULD_CRASH returns > 0, cuz < 0 means process interrupted
