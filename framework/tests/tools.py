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
def runTests(argv, app_name, moose_dir):
  host_name = gethostname()
  if host_name == 'service0' or host_name == 'service1':
    print 'Testing not supported on Icestorm head node'
    sys.exit(0)

  harness = TestHarness(argv, app_name, moose_dir)
  harness.findAndRunTests()


class TestHarness:

  def __init__(self, argv, app_name, moose_dir):
    self.test_table = []

    self.num_passed = 0
    self.num_failed = 0
    self.num_skipped = 0
    self.host_name = gethostname()
    self.moose_dir = os.path.abspath(moose_dir) + '/'
    #Assume libmesh is a peer directory to MOOSE if not defined
    if os.environ.has_key("LIBMESH_DIR"):
      self.libmesh_dir = os.environ['LIBMESH_DIR']
    else:
      self.libmesh_dir = self.moose_dir + '../libmesh'
    self.file = None

    self.checks = {}
    self.checks[PLATFORM] = getPlatforms()
    self.checks[COMPILER] = getCompilers(self.libmesh_dir)
    self.checks[PETSC_VERSION] = getPetscVersion(self.libmesh_dir)
    self.checks[MESH_MODE] = getParmeshOption(self.libmesh_dir)

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
    command = ''

    if test[PARALLEL] > 1:
      command = 'mpiexec -n ' + test[PARALLEL] + ' -host ' + self.host_name + ' ' + self.executable + ' -i ' + test[INPUT] + ' ' +  ' '.join(test[CLI_ARGS])
    else:
      command = self.executable + ' -i ' + test[INPUT] + ' ' + ' '.join(test[CLI_ARGS])
    if self.options.scaling and test[SCALE_REFINE] > 0:
      command += ' -r ' + str(test[SCALE_REFINE])
    return command

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
    # Are we running only tests in a specific group?
    if self.options.group <> 'ALL' and self.options.group not in test[GROUP]:
      return False

    if self.options.not_group <> '' and self.options.not_group in test[GROUP]:
      return False

    # Check for skipped tests
    if type(test[SKIP]) is bool:
      # Backwards compatible (no reason)
      if test[SKIP]:
        self.handleTestResult(test, '', 'skipped')
        return False
    elif test[SKIP] != '':
      # We might want to trim the string so it formats nicely
      if len(test[SKIP]) >= TERM_COLS - (len(test)+21):
        test_reason = (test[SKIP])[:(TERM_COLS - (len(test)+24))] + '...'
      else:
        test_reason = test[SKIP]
      self.handleTestResult(test, '', 'skipped (' + test_reason + ')')
      return False

    checks = [PLATFORM, COMPILER, PETSC_VERSION, MESH_MODE]
    for check in checks:
      test_platforms = set()
      for x in test[check]:
        test_platforms.add(x)
      if not len(test_platforms.intersection(self.checks[check])):
        self.handleTestResult(test, '', 'skipped (' + \
          re.sub(r'\[|\]', '', check).upper() + '!=' + ', '.join(test[check]) + ')')
        return False

    # Check for heavy tests
    if test[HEAVY] and not self.options.heavy_tests:
      self.handleTestResult(test, '', 'skipped (HEAVY)')
      return False

    return True

  ## Finish the test by inspecting the raw output
  def testOutputAndFinish(self, test, retcode, output):
    reason = ''
    caveats = []

    # Expected errors and assertions might do a lot of things including crash so we
    # will handle them seperately
    if test[EXPECT_ERR] != None:
      if not self.checkExpectError(output, test[EXPECT_ERR]):
        reason = 'NO EXPECTED ERR'
    elif test[EXPECT_OUT] != None:
      out_ok = self.checkExpectError(output, test[EXPECT_OUT])
      if (out_ok and retcode != 0):
        reason = 'OUT FOUND BUT CRASH'
      elif (not out_ok):
        reason = 'NO EXPECTED OUT'
    elif test[EXPECT_ASSERT] != None:
      if self.options.method == 'dbg':  # Only check asserts in debug mode
        if not self.checkExpectError(output, test[EXPECT_ASSERT]):
          reason = 'NO EXPECTED ASSERT'
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
        # There may be other reasons we don't run exodiff (this is the only one for now
        if self.options.scaling and test[SCALE_REFINE]:
          caveats.append('SCALED')
        else:
          for file in test[EXODIFF]:
            custom_cmp = ''
            old_floor = ''
            if test[CUSTOM_CMP] != None:
               custom_cmp = ' -f ' + os.path.join(test[TEST_DIR], test[CUSTOM_CMP])
            if test[USE_OLD_FLOOR]:
               old_floor = ' -use_old_floor'
            command = self.moose_dir + 'contrib/exodiff/exodiff -m' + custom_cmp + ' -F ' + str(test[ABS_ZERO]) + old_floor +  ' -t ' + str(test[REL_ERR]) \
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

    if reason == '':
      # It ran OK but is this test set to be skipped on any platform, compiler, so other reason?
# TODO: Refactor this mess
      if not 'ALL' in test[PLATFORM]:
        caveats.append(', '.join(test[PLATFORM]))
      if not 'ALL' in test[COMPILER]:
        caveats.append(', '.join(test[COMPILER]))
      if not 'ALL' in test[PETSC_VERSION]:
        caveats.append(', '.join(test[PETSC_VERSION]))
      if not 'ALL' in test[MESH_MODE]:
        caveats.append(', '.join(test[MESH_MODE]))
      if len(caveats):
        result = '[' + ', '.join(caveats) + '] OK'
      else:
        result = 'OK'
    else:
      result = 'FAILED (%s)' % reason
    self.handleTestResult(test, output, result)

  def getTiming(self, output):
    time = ''
    m = re.search(r"Active time=(\S+)", output)
    if m != None:
      return m.group(1)

  def checkExpectError(self, output, expect_error):
    if re.search(expect_error, output, re.MULTILINE) == None:
      #print "%" * 100, "\nExpect Error Pattern not found:\n", expect_error, "\n", "%" * 100, "\n"
      return False
    else:
      return True

  ## Update global variables and print output based on the test result
  # Containing OK means it passed, skipped means skipped, anything else means it failed
  def handleTestResult(self, test, output, result):
    timing = ''

    if self.options.timing:
      timing = self.getTiming(output)

    self.test_table.append( (test, output, result, timing) )
    print printResult(test[TEST_NAME], result, timing, self.options)

    if result.find('OK') != -1:
      self.num_passed += 1
    elif 'skipped' in result:
      self.num_skipped += 1
    else:
      self.num_failed += 1

    if self.options.verbose or ('FAILED' in result and not self.options.quiet):
      print output

    if not 'skipped' in result:
      if self.options.file:
        self.file.write(printResult( test[TEST_NAME], result, timing, self.options, color=False) + '\n')
        self.file.write(output)

      if self.options.sep_files or (self.options.fail_files and 'FAILED' in result) or (self.options.ok_files and result.find('OK') != -1):
        fname = os.path.join(test[TEST_DIR], test[TEST_NAME] + '.' + result[:6] + '.txt')
        f = open(fname, 'w')
        f.write(printResult( test[TEST_NAME], result, timing, self.options, color=False) + '\n')
        f.write(output)
        f.close()

  # Write the app_name to a file, if the tests passed
  def writeState(self, app_name):
    # If we encounter bitten_status_moose environment, build a line itemized list of applications which passed their tests
    if os.environ.has_key("BITTEN_STATUS_MOOSE"):
      result_file = open(os.path.join(self.moose_dir, 'test_results.log'), 'a')
      result_file.write(str(os.path.split(app_name)[1][:-4]) + '\n')
      result_file.close()

  # Print final results, close open files, and exit with the correct error code
  def cleanupAndExit(self):
    # Print the results table again if a bunch of output was spewed to the screen between
    # tests as they were running
    if self.options.verbose or (self.num_failed != 0 and not self.options.quiet):
      print '\n\nFinal Test Results:\n' + ('-' * (TERM_COLS-1))
      for (test, output, result, timing) in self.test_table:
        print printResult(test[TEST_NAME], result, timing, self.options)

    time = clock() - self.start_time
    print '-' * (TERM_COLS-1)
    print 'Ran %d tests in %.1f seconds' % (self.num_passed+self.num_failed, time)

    if self.num_passed:
      summary = '<g>%d passed</g>'
    else:
      summary = '<b>%d passed</b>'
    summary += ', <b>%d skipped</b>, '
    if self.num_failed:
      summary += '<r>%d FAILED</r>'
    else:
      summary += '<b>%d failed</b>'
    print colorify( summary % (self.num_passed, self.num_skipped, self.num_failed), self.options, html=True )

    if self.file:
      self.file.close()

    if self.num_failed == 0:
      self.writeState(self.executable)
      sys.exit(0)
    else:
      sys.exit(1)

  def initialize(self, argv, app_name):
    self.parseCLArgs(argv)

    # Initialize the parallel runner with how many tests to run in parallel
    self.runner = RunParallel(self, self.options.jobs, self.options.load)

    ## Save executable-under-test name to self.executable
    self.executable = os.getcwd() + '/' + app_name + '-' + self.options.method

    # Emulate the standard Nose RegEx for consistency
    self.test_match = re.compile(r"(?:^|\b|[_-])[Tt]est")

    # Save the output dir since the current working directory changes during tests
    self.output_dir = os.path.join(os.path.abspath(os.path.dirname(sys.argv[0])), self.options.output_dir)

    # Create the output dir if they ask for it. It is easier to ask for forgiveness than permission
    if self.options.output_dir:
      try:
        os.makedirs(self.output_dir)
      except OSError, ex:
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
    parser.add_option('-j', '--jobs', action='store', type='int', dest='jobs', default=1,
                      help='run test binaries in parallel')
    parser.add_option("-c", "--no-color", action="store_false", dest="colored", default=True,
                      help="Do not show colored output")
    parser.add_option('--heavy', action='store_true', dest='heavy_tests', default=False,
                      help='Run normal tests and tests marked with HEAVY : True')
    parser.add_option('-g', '--group', action='store', type='string', dest='group', default='ALL',
                      help='Run only tests in the named group')
    parser.add_option('--not_group', action='store', type='string', dest='not_group', default='',
                      help='Run only tests NOT in the named group')
    parser.add_option('--dofs', action='store', dest='dofs', default=0,
                      help='This option is for automatic scaling which is not currently implemented in MOOSE 2.0')
    parser.add_option('-l', '--load-average', action='store', type='float', dest='load', default=64.0,
                      help='Do not run additional tests if the load average is at least LOAD')
    parser.add_option('-t', '--timing', action='store_true', dest='timing', default=False,
                      help="Report Timing information for passing tests")
    parser.add_option('-s', '--scale', action='store_true', dest='scaling', default=False,
                      help="Scale problems that have SCALE_REFINE set")

    outputgroup = OptionGroup(parser, 'Output Options', 'These options control the output of the test harness. The sep-files options write output to files named test_name.TEST_RESULT.txt. All file output will overwrite old files')
    outputgroup.add_option('-v', '--verbose', action='store_true', dest='verbose', default=False, help='show the output of every test that fails')
    outputgroup.add_option('-q', '--quiet', action='store_true', dest='quiet', default=False, help='only show the result of every test, don\'t show test output even if it fails')
    outputgroup.add_option('-o', '--output-dir', action='store', dest='output_dir', default='', metavar='DIR', help='Save all output files in the directory, and create it if necessary')
    outputgroup.add_option('-f', '--file', action='store', dest='file', default=None, metavar='FILE', help='Write verbose output of each test to FILE and quiet output to terminal')
    outputgroup.add_option('-x', '--sep-files', action='store_true', dest='sep_files', default=False, metavar='FILE', help='Write the output of each test to a separate file. Only quiet output to terminal. This is equivalant to \'--sep-files-fail --sep-files-ok\'')
    outputgroup.add_option('--sep-files-ok', action='store_true', dest='ok_files', default=False, metavar='FILE', help='Write the output of each passed test to a separate file')
    outputgroup.add_option('-a', '--sep-files-fail', action='store_true', dest='fail_files', default=False, metavar='FILE', help='Write the output of each FAILED test to a separate file. Only quiet output to terminal.')
    outputgroup.add_option("--store-timing", action="store_true", dest="time", default=False, help="Store timing in the database (Currently not implemented)")
    outputgroup.add_option("-r", "--revision", action="store", dest="revision", help="REQUIRED: the current revision (Currently not implemented)")


    parser.add_option_group(outputgroup)

    (self.options, self.tests) = parser.parse_args(argv[1:])
    self.checkAndUpdateCLArgs()

  ## Called after options are parsed from the command line
  # Exit if options don't make any sense, print warnings if they are merely weird
  def checkAndUpdateCLArgs(self):
    opts = self.options
    if opts.output_dir and not (opts.file or opts.sep_files or opts.fail_files or opts.ok_files):
      print 'WARNING: --output-dir is specified but no output files will be saved, use -f or a --sep-files option'
    if opts.group == opts.not_group:
      print 'ERROR: The group and not_group options cannot specify the same group'
      sys.exit(1)

    # Update any keys from the environment as necessary
    if not self.options.method:
      if os.environ.has_key('METHOD'):
        self.options.method = os.environ['METHOD']
      else:
        self.options.method = 'opt'

# Notes:
# SHOULD_CRASH returns > 0, cuz < 0 means process interrupted
