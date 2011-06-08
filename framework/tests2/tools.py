import os, sys, re, inspect, types, errno, pprint
from socket import gethostname
from optparse import OptionParser, OptionGroup
#from optparse import OptionG
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
    self.test_table = []

    self.num_passed = 0
    self.num_failed = 0
    self.num_skipped = 0

    self.file = None

    self.initialize(argv, app_name)

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
                pp = pprint.PrettyPrinter()
                
                # insert default values where none provided
                testname = module_name + '.' + test_name
                test = DEFAULTS.copy()

                # nested dictionaries have to be copied seperately
                for (key, value) in test.iteritems():
                  if type(value) == dict:
                    my_dict = DEFAULTS[key].copy()
                    if key in test_opts:
                      my_dict.update(test_opts[key])
                      test[key] = my_dict
                      del test_opts[key]
                
                # Now update all the base level keys
                test.update(test_opts)
                test.update( { TEST_NAME : testname, TEST_DIR : test_dir } )

                if test[PREREQ] != None:
                  test[PREREQ] = module_name + '.' + test[PREREQ]

                if self.checkIfRunTest(test):
                  self.prepareTest(test)
                  execute = self.createCommand(test)

                  # This method spawns another process and allows this loop to continue looking for tests
                  # RunParallel will call self.testOutputAndFinish when the test has completed running
                  # This method will block when the maximum allowed parallel processes are running
                  self.runner.run(test, execute)
                                
            os.chdir(saved_cwd)
            sys.path.pop()

    # Wait for all tests to finish
    self.runner.join()
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
    platform = set()
    platform.add('ALL')
    platform.add((os.uname()[0]).upper())
    if test[PLATFORM] not in platform:
      self.handleTestResult(test, '', 'skipped')
    return True

  ## Finish the test by inspecting the raw output 
  def testOutputAndFinish(self, test, retcode, output):
    reason = ''

    # Expected errors might do a lot of things including crash so we
    # will handle them seperately
    if test[EXPECT_ERR] != None:
      if not self.checkExpectError(output, test[EXPECT_ERR]):
        reason = 'NO EXPECTED ERR'
    else:
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
          custom_cmp = ''
          if test[EXO_OPTIONS][CUSTOM_CMP] != None:
             custom_cmp = ' -f ' + os.path.join(test[TEST_DIR], test[EXODIFF][CUSTOM_CMP])
          command = 'exodiff -m' + custom_cmp + ' -F ' + str(test[EXO_OPTIONS][ABS_ZERO]) + ' -use_old_floor -t ' + str(test[EXO_OPTIONS][REL_ERR]) \
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

  def checkExpectError(self, output, expect_error):
    if re.search(expect_error, output, re.IGNORECASE) == None:
      #print "%" * 100, "\nExpect Error Pattern not found:\n", expect_error, "\n", "%" * 100, "\n"
      return False
    else:
      return True

  ## Update global variables and print output based on the test result
  # OK means it passed, skipped means skipped, anything else means it failed
  def handleTestResult(self, test, output, result):
    self.test_table.append( (test, output, result) )
    print printResult(test[TEST_NAME], result, self.options)

    if result == 'OK':
      self.num_passed += 1
    elif result == 'skipped':
      self.num_skipped += 1
    else:
      self.num_failed += 1

    if self.options.verbose or ('FAILED' in result and not self.options.quiet):
      print output

    if result != 'skipped':
      if self.options.file:
        self.file.write(printResult( test[TEST_NAME], result, self.options, color=False) + '\n')
        self.file.write(output)

      if self.options.sep_files or (self.options.fail_files and 'FAILED' in result) or (self.options.ok_files and result == 'OK'):
        fname = os.path.join(self.output_dir, test[TEST_NAME] + '.' + result[:6] + '.txt')
        f = open(fname, 'w')
        f.write(printResult( test[TEST_NAME], result, self.options, color=False) + '\n')
        f.write(output)
        f.close()

  # Print final results, close open files, and exit with the correct error code
  def cleanupAndExit(self):
    # Print the results table again if a bunch of output was spewed to the screen between
    # tests as they were running
    if self.options.verbose or (self.num_failed != 0 and not self.options.quiet):
      print '\n\nFinal Test Results:\n' + ('-' * 80)
      for (test, output, result) in self.test_table:
        print printResult(test[TEST_NAME], result, self.options)

    time = clock() - self.start_time
    print '-' * 80
    print 'Ran %d tests in %.1f seconds' % (self.num_passed+self.num_failed, time)

    summary = '<g>%d passed</g>' if self.num_passed else '<b>%d passed</b>'
    summary += ', <b>%d skipped</b>, '
    summary += '<r>%d FAILED</r>' if self.num_failed else '<b>%d failed</b>'
    print colorify( summary % (self.num_passed, self.num_skipped, self.num_failed), self.options, html=True )

    if self.file:
      self.file.close()

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

    # Save the output dir since the current working directory changes during tests
    self.output_dir = os.path.join(os.path.abspath(os.path.dirname(sys.argv[0])), self.options.output_dir)

    # Create the output dir if they ask for it. It is easier to ask for forgiveness than permission
    if self.options.output_dir:
      try:
        os.makedirs(self.output_dir)
      except OSError as ex:
        if ex.errno == errno.EEXIST: pass
        else: raise

    # Open the file to redirect output to and set the quiet option for file output
    if self.options.file:
      self.file = open(os.path.join(self.output_dir, self.options.file), 'w')
    if self.options.file or self.options.fail_files or self.options.sep_files:
      self.options.quiet = True


  ## Parse command line options and assign them to self.options
  def parseCLArgs(self, argv):
    parser = OptionParser()
    parser.add_option('--opt', action='store_const', dest='method', const='opt', help='test the app_name-opt binary')
    parser.add_option('--dbg', action='store_const', dest='method', const='dbg', help='test the app_name-dbg binary')
    parser.add_option('--dev', action='store_const', dest='method', const='dev', help='test the app_name-dev binary')
    parser.add_option('-j', '--jobs', action='store', type='int', dest='jobs', default=1, help='run test binaries in parallel')
    parser.add_option("-c", "--no-color", action="store_false", dest="colored", default=True, help="Do not show colored output")

    outputgroup = OptionGroup(parser, 'Output Options', 'These options control the output of the test harness. The sep-files options write output to files named test_name.TEST_RESULT.txt. All file output will overwrite old files')
    outputgroup.add_option('-v', '--verbose', action='store_true', dest='verbose', default=False, help='show the output of every test that fails')
    outputgroup.add_option('-q', '--quiet', action='store_true', dest='quiet', default=False, help='only show the result of every test, don\'t show test output even if it fails')
    outputgroup.add_option('-o', '--output-dir', action='store', dest='output_dir', default='', metavar='DIR', help='Save all output files in the directory, and create it if necessary')
    outputgroup.add_option('-f', '--file', action='store', dest='file', default=None, metavar='FILE', help='Write verbose output of each test to FILE and quiet output to terminal')
    outputgroup.add_option('-s', '--sep-files', action='store_true', dest='sep_files', default=False, metavar='FILE', help='Write the output of each test to a separate file. Only quiet output to terminal. This is equivalant to \'--sep-files-fail --sep-files-ok\'')
    outputgroup.add_option('--sep-files-ok', action='store_true', dest='ok_files', default=False, metavar='FILE', help='Write the output of each passed test to a separate file')
    outputgroup.add_option('-a', '--sep-files-fail', action='store_true', dest='fail_files', default=False, metavar='FILE', help='Write the output of each FAILED test to a separate file. Only quiet output to terminal.')

    parser.add_option_group(outputgroup)

    (self.options, self.tests) = parser.parse_args(argv[1:])
    self.checkCLArgs()

  ## Called after options are parsed from the command line
  # Exit if options don't make any sense, print warnings if they are merely weird
  def checkCLArgs(self):
    opts = self.options
    if opts.output_dir and not (opts.file or opts.sep_files or opts.fail_files or opts.ok_files):
      print 'WARNING: --output-dir is specified but no output files will be saved, use -f or a --sep-files option'


# Notes:
# SHOULD_CRASH returns > 0, cuz < 0 means process interrupted
