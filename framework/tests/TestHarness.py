import os, sys, re, inspect, types, errno, pprint
import ParseGetPot
from socket import gethostname
from options import *
from util import *
from time import sleep
from RunParallel import RunParallel
from CSVDiffer import CSVDiffer
from Tester import Tester

from optparse import OptionParser, OptionGroup
#from optparse import OptionG
from timeit import default_timer as clock

class TestHarness:
  def __init__(self, argv, app_name, moose_dir):
    self.testers = {}   # The registered 'Tester' types
    self.input_file_name = 'tests'  # This is the file we look for, when looking for test specifications.
    self.test_table = []
    self.num_passed = 0
    self.num_failed = 0
    self.num_skipped = 0
    self.host_name = gethostname()
    self.moose_dir = os.path.abspath(moose_dir) + '/'
    # Assume libmesh is a peer directory to MOOSE if not defined
    if os.environ.has_key("LIBMESH_DIR"):
      self.libmesh_dir = os.environ['LIBMESH_DIR']
    else:
      self.libmesh_dir = self.moose_dir + '../libmesh'
    self.file = None

    # Parse arguments
    self.parseCLArgs(argv)

    self.checks = {}
    self.checks[PLATFORM] = getPlatforms()
    self.checks[COMPILER] = getCompilers(self.libmesh_dir)
    self.checks[PETSC_VERSION] = getPetscVersion(self.libmesh_dir)
    self.checks[MESH_MODE] = getParmeshOption(self.libmesh_dir)
    self.checks[LIBRARY_MODE] = getSharedOption(self.libmesh_dir)

    method = set()
    method.add('ALL')
    method.add(self.options.method.upper())
    self.checks[METHOD] = method

    self.initialize(argv, app_name)

  def findAndRunTests(self):
    self.preRun()
    self.start_time = clock()
    for dirpath, dirnames, filenames in os.walk(os.getcwd()):
      if (self.test_match.search(dirpath)):
        for file in filenames:
          # See if there were other arguments (test names) passed on the command line
          if file == self.input_file_name or file[-2:] == 'py' and self.test_match.search(file):
            saved_cwd = os.getcwd()
            sys.path.append(os.path.abspath(dirpath))
            os.chdir(dirpath)
            if file == self.input_file_name:  # New GetPot file formatted test
              tests = self.parseGetPotTestFormat(file)
            elif file[-2:] == 'py' and self.test_match.search(file): # Legacy file formatted test
              tests = self.parseLegacyTestFormat(file)

            # Go through the list of test specs and run them
            for test in tests:

	      # TODO: Refactor all this - these should be methods on the Tester not part of the TestHarness
              if self.checkIfRunTest(test):
                self.augmentTestSpecs(test)

		# Get the validParams for the requested Tester type
		params = self.getValidParams(test[TYPE])

		# Update the params object using the raw test specs dictionary
		params = self.populateParams(params, test)

                # Build the requested Tester object and run
                tester = self.create(test[TYPE], params)
                command = tester.getCommand(self.options)

                tester.prepare()

                # This method spawns another process and allows this loop to continue looking for tests
                # RunParallel will call self.testOutputAndFinish when the test has completed running
                # This method will block when the maximum allowed parallel processes are running
                self.runner.run(tester, command)
              else: # This job is skipped - notify the runner
                self.runner.jobSkipped(test[TEST_NAME])

            os.chdir(saved_cwd)
            sys.path.pop()

    # Wait for all tests to finish
    self.runner.join()
    self.cleanupAndExit()

  def parseGetPotTestFormat(self, filename):
    tests = []
    test_dir = os.path.abspath(os.path.dirname(filename))

    # Filter tests that we want to run
    # Under the new format, we will filter based on directory not filename since it is fixed
    will_run = False
    if len(self.tests) == 0:
      will_run = True
    else:
      for item in self.tests:
        if test_dir.find(item) > -1:
          will_run = True
    if not will_run:
      return tests

    try:
      data = ParseGetPot.readInputFile(filename)
    except e:        # ParseGetPot class
      print "Parse Error: " + test_dir + "/" + filename
      return tests

    # We expect our root node to be called "Tests"
    if 'Tests' in data.children:
      tests_node = data.children['Tests']

      for testname, test_node in tests_node.children.iteritems():
        test = DEFAULTS.copy()

        # Get relative path to test[TEST_DIR]
        executable_path = os.path.dirname(self.executable)
        relative_path = test_dir.replace(executable_path, '')

        # Now update all the base level keys
        for key, value in test_node.params.iteritems():
          if key in test and type(test[key]) == list:
            test[key] = value.split(' ')
          else:
	    if re.match('".*"', value):  # Strip quotes
	      test[key] = value[1:-1]
	    else:
              test[key] = value

        if TYPE not in test:
          print "Type missing in " + test_dir + filename
          sys.exit(1)

	# TODO: In progress formatting
#        formatted_name = relative_path + '.' + testname
        formatted_name = relative_path.replace('/tests/', '') + '.' + testname
#	formatted_name = '/'.join(relative_path.split('/')[1:]) + '.' + testname
#        formatted_name += ' (' + relative_path
#        if test[INPUT] != 'input':  # See if a testname was provided
#          formatted_name += '/' + test[INPUT]
#        formatted_name += ')'

        test.update( { TEST_NAME : formatted_name, TEST_DIR : test_dir, RELATIVE_PATH : relative_path } )

        if test[PREREQ] != None:
          if type(test[PREREQ]) != list:
            print "Option 'PREREQ' needs to be of type list in " + test[TEST_NAME]
            sys.exit(1)
          test[PREREQ] = [relative_path.replace('/tests/', '') + '.' + item for item in test[PREREQ]]

        # Build a list of test specs (dicts) to return
        tests.append(test)
    return tests

  def parseLegacyTestFormat(self, filename):
    # dynamically load the module
    module_name = filename[:-3]   # Always a python file (*.py)
    module = __import__(module_name)
    test_dir = os.path.dirname(module.__file__)
    tests = []

    for test_name, test_opts in inspect.getmembers(module):
      if isinstance(test_opts, types.DictType) and self.test_match.search(test_name):

        if filename == 'periodic_bc_test.py':
	  print "  [./" + test_name + "]"
          print "    type = 'Exodiff'"
	  for key, value in test_opts.items():
            print "    " + key + " = '", value, "'"
          print "  [../]\n"

        # insert default values where none provided
        testname = module_name + '.' + test_name

        # Filter tests that we want to run
        will_run = False
        if len(self.tests) == 0:
          will_run = True
        else:
          for item in self.tests:
            pos = item.find(".")
            # Does the passed in argument have a "dot"?
            if (pos > -1 and item == testname) or (pos == -1 and item == module_name):
              will_run = True

        if not will_run:
          continue

        test = DEFAULTS.copy()

        # Get relative path to test[TEST_DIR]
        relative_path = test_dir.replace(self.executable.split(self.executable.split('/').pop())[0], '')

        # Now update all the base level keys
        test.update(test_opts)

	# Backwards compatibility
        if len(test[CSVDIFF]) > 0 and TYPE not in test:
          print 'CSVDIFF is deprecated in the legacy test format unless "type" is supplied. Please convert to the new format:\n' + test_dir + '/' + filename
          sys.exit(1)
	if (test[EXPECT_ERR] != None or test[EXPECT_ASSERT] != None or test[SHOULD_CRASH] == True) and not TYPE in test:
	  test[TYPE] = 'RunException'

        test.update( { TEST_NAME : testname, TEST_DIR : test_dir, RELATIVE_PATH : relative_path } )
	if TYPE not in test:
	  test.update( { TYPE : "Exodiff" } )

        if test[PREREQ] != None:
          if type(test[PREREQ]) != list:
            print "Option 'PREREQ' needs to be of type list in " + module_name + '.' + test[TEST_NAME]
            sys.exit(1)
          test[PREREQ] = [module_name + '.' + item for item in test[PREREQ]]

        # Build a list of test specs (dicts) to return
        tests.append(test)
    return tests

  def augmentTestSpecs(self, test):
    test[EXECUTABLE] = self.executable
    test[HOSTNAME] = self.host_name

  # If the test is not to be run for any reason, print skipped as the result and return False,
  # otherwise return True
  def checkIfRunTest(self, test):
    # Are we running only tests in a specific group?
    if self.options.group <> 'ALL' and self.options.group not in test[GROUP]:
      return False
    if self.options.not_group <> '' and self.options.not_group in test[GROUP]:
      return False

    # Store regexp for matching tests if --re is used
    if self.options.reg_exp:
      match_regexp = re.compile(self.options.reg_exp)

    # If --re then only test matching regexp. Needs to run before other SKIP methods
    if self.options.reg_exp and not match_regexp.search(test[TEST_NAME]):
      return False

    # Check for deleted tests
    if test[DELETED]:
      if self.options.extra_info:
        # We might want to trim the string so it formats nicely
        if len(test[DELETED]) >= TERM_COLS - (len(test)+21):
          test_reason = (test[DELETED])[:(TERM_COLS - (len(test)+24))] + '...'
        else:
          test_reason = test[DELETED]
        self.handleTestResult(test, '', 'deleted (' + test_reason + ')')
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
    # If were testing for SCALE_REFINE, then only run tests with a SCALE_REFINE set
    elif self.options.store_time and test[SCALE_REFINE] == 0:
      return False
    # If we're testing with valgrind, then skip tests that require parallel or threads
    elif self.options.enable_valgrind and (test[MIN_PARALLEL] > 1 or test[MIN_THREADS] > 1):
      self.handleTestResult(test, '', 'skipped (Valgrind requires serial)')
      return False

    # Check for PETSc versions
    (petsc_status, logic_reason, petsc_version) = self.checkPetscVersion(test)
    if not petsc_status:
      self.handleTestResult(test, '', 'skipped (PETSc version ' + \
                              str(self.checks[PETSC_VERSION]) + ' ' + logic_reason + ' ' + petsc_version + ')')
      return False

    # PETSc is being explicitly checked above
    checks = [PLATFORM, COMPILER, MESH_MODE, METHOD, LIBRARY_MODE]
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

  # Break down petsc version logic in a new define
  # TODO: find a way to eval() logic instead
  def checkPetscVersion(self, test):
    # If any version of petsc works, return true immediately
    if 'ALL' in set(test[PETSC_VERSION]):
      return (True, None, None)
    # Iterate through petsc versions in test[PETSC_VERSION] and match it against check[PETSC_VERSION]
    for petsc_version in test[PETSC_VERSION]:
      logic, version = re.search(r'(.*?)(\d\S+)', petsc_version).groups()
      # Exact match
      if logic == '' or logic == '=':
        if version == self.checks[PETSC_VERSION]:
          return (True, None, version)
        else:
          return (False, '!=', version)
      # Logical match
      if logic == '>' and self.checks[PETSC_VERSION][0:3] > version[0:3]:
        return (True, None, version)
      elif logic == '>=' and self.checks[PETSC_VERSION][0:3] >= version[0:3]:
        return (True, None, version)
      elif logic == '<' and self.checks[PETSC_VERSION][0:3] < version[0:3]:
        return (True, None, version)
      elif logic == '<=' and self.checks[PETSC_VERSION][0:3] <= version[0:3]:
        return (True, None, version)
    return (False, logic, version)

  ## Finish the test by inspecting the raw output
  def testOutputAndFinish(self, tester, retcode, output, start=0, end=0):
    caveats = []
    test = tester.specs  # Need to refactor

    if 'CAVEATS' in test:
      caveats = test['CAVEATS']

    (reason, output) = tester.processResults(self.moose_dir, retcode, self.options, output)
#    print tester.processResults(self.moose_dir, retcode, self.options, output)

    if self.options.scaling and test[SCALE_REFINE]:
      caveats.append('SCALED')

    # if still no errors, check other files (just for existence)
#    if reason == '':
#      for file in test[CHECK_FILES]:
#        if not os.path.isfile(os.path.join(test[TEST_DIR], file)):
#          reason = 'MISSING FILES'
#          break

    did_pass = True
    if reason == '':
      # It ran OK but is this test set to be skipped on any platform, compiler, so other reason?
      if self.options.extra_info:
        checks = [PLATFORM, COMPILER, PETSC_VERSION, MESH_MODE, METHOD, LIBRARY_MODE]
        for check in checks:
          if not 'ALL' in test[check]:
            caveats.append(', '.join(test[check]))
      if len(caveats):
        result = '[' + ', '.join(caveats) + '] OK'
      else:
        result = 'OK'
    else:
      result = 'FAILED (%s)' % reason
      did_pass = False
    self.handleTestResult(tester.specs, output, result, start, end)
    return did_pass

  def getTiming(self, output):
    time = ''
    m = re.search(r"Active time=(\S+)", output)
    if m != None:
      return m.group(1)

  def checkExpectError(self, output, expect_error):
    if re.search(expect_error, output, re.MULTILINE | re.DOTALL) == None:
      #print "%" * 100, "\nExpect Error Pattern not found:\n", expect_error, "\n", "%" * 100, "\n"
      return False
    else:
      return True

  ## Update global variables and print output based on the test result
  # Containing OK means it passed, skipped means skipped, anything else means it failed
  def handleTestResult(self, specs, output, result, start=0, end=0):
    timing = ''

    if self.options.timing:
      timing = self.getTiming(output)

    self.test_table.append( (specs, output, result, timing, start, end) )

    self.postRun(specs, timing)

    if self.options.show_directory:
      print printResult(specs[RELATIVE_PATH] + '/' + specs[TEST_NAME], result, timing, start, end, self.options)
    else:
      print printResult(specs[TEST_NAME], result, timing, start, end, self.options)

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
        if self.options.show_directory:
          self.file.write(printResult( specs[RELATIVE_PATH] + '/' + specs[TEST_NAME], result, timing, start, end, self.options, color=False) + '\n')
          self.file.write(output)
        else:
          self.file.write(printResult( specs[TEST_NAME], result, timing, start, end, self.options, color=False) + '\n')
          self.file.write(output)

      if self.options.sep_files or (self.options.fail_files and 'FAILED' in result) or (self.options.ok_files and result.find('OK') != -1):
        fname = os.path.join(specs[TEST_DIR], specs[TEST_NAME] + '.' + result[:6] + '.txt')
        f = open(fname, 'w')
        f.write(printResult( specs[TEST_NAME], result, timing, start, end, self.options, color=False) + '\n')
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
      for (test, output, result, timing, start, end) in self.test_table:
        if self.options.show_directory:
          print printResult(test[RELATIVE_PATH] + '/' + test[TEST_NAME], result, timing, start, end, self.options)
        else:
          print printResult(test[TEST_NAME], result, timing, start, end, self.options)

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
    # Initialize the parallel runner with how many tests to run in parallel
    self.runner = RunParallel(self, self.options.jobs, self.options.load)

    ## Save executable-under-test name to self.executable
    self.executable = os.getcwd() + '/' + app_name + '-' + self.options.method

    # Check for built application
    if not os.path.exists(self.executable):
      print 'Application not found: ' + str(self.executable)
      sys.exit(1)

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
    parser.add_option('-e', action="store_true", dest="extra_info", default=False,
    	              help='Display "extra" information including all caveats and deleted tests')
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
    parser.add_option('--dbfile', action='store', dest='dbFile', default=0,
                      help='Location to timings data base file. If not set, assumes $HOME/timingDB/timing.sqlite')
    parser.add_option('-l', '--load-average', action='store', type='float', dest='load', default=64.0,
                      help='Do not run additional tests if the load average is at least LOAD')
    parser.add_option('-t', '--timing', action='store_true', dest='timing', default=False,
                      help="Report Timing information for passing tests")
    parser.add_option('-s', '--scale', action='store_true', dest='scaling', default=False,
                      help="Scale problems that have SCALE_REFINE set")
    parser.add_option('--libmesh_dir', action="store", type='string', dest="libmesh_dir", default='',
                      help="Currently only needed for bitten code coverage")
    parser.add_option('--parallel', '-p', action="store", type='int', dest="parallel",
                      help="Number of processors to use when running mpiexec")
    parser.add_option('--threads', action="store", type='int', dest="nthreads",default=1,
                      help="Number of threads to use when running mpiexec")
    parser.add_option('-d', action='store_true', dest='debug_harness', default=False, help='Turn on Test Harness debugging')
    parser.add_option('--valgrind', action='store_true', dest='enable_valgrind', default=False, help='Enable Valgrind')
    parser.add_option('--re', action='store', type='string', dest='reg_exp', default='', help='Run tests that match --re=regular_expression')

    outputgroup = OptionGroup(parser, 'Output Options', 'These options control the output of the test harness. The sep-files options write output to files named test_name.TEST_RESULT.txt. All file output will overwrite old files')
    outputgroup.add_option('-v', '--verbose', action='store_true', dest='verbose', default=False, help='show the output of every test that fails')
    outputgroup.add_option('-q', '--quiet', action='store_true', dest='quiet', default=False, help='only show the result of every test, don\'t show test output even if it fails')
    outputgroup.add_option('--show-directory', action='store_true', dest='show_directory', default=False, help='Print test directory path in out messages')
    outputgroup.add_option('-o', '--output-dir', action='store', dest='output_dir', default='', metavar='DIR', help='Save all output files in the directory, and create it if necessary')
    outputgroup.add_option('-f', '--file', action='store', dest='file', default=None, metavar='FILE', help='Write verbose output of each test to FILE and quiet output to terminal')
    outputgroup.add_option('-x', '--sep-files', action='store_true', dest='sep_files', default=False, metavar='FILE', help='Write the output of each test to a separate file. Only quiet output to terminal. This is equivalant to \'--sep-files-fail --sep-files-ok\'')
    outputgroup.add_option('--sep-files-ok', action='store_true', dest='ok_files', default=False, metavar='FILE', help='Write the output of each passed test to a separate file')
    outputgroup.add_option('-a', '--sep-files-fail', action='store_true', dest='fail_files', default=False, metavar='FILE', help='Write the output of each FAILED test to a separate file. Only quiet output to terminal.')
    outputgroup.add_option("--store-timing", action="store_true", dest="store_time", default=False, help="Store timing in the SQL database: $HOME/timingDB/timing.sqlite A parent directory (timingDB) must exist.")
    outputgroup.add_option("--revision", action="store", dest="revision", help="The current revision being tested. Required when using --store-timing.")


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
    if opts.store_time and not (opts.revision):
      print 'ERROR: --store-timing is specified but no revision'
      sys.exit(1)
    if opts.store_time and not (opts.timing or opts.scaling):
      opts.timing = True
      opts.scaling = True
    if opts.enable_valgrind and (opts.parallel > 1 or opts.nthreads > 1):
      print 'ERROR: --parallel and/or --threads can not be used with --valgrind'
      sys.exit(1)

    # Update any keys from the environment as necessary
    if not self.options.method:
      if os.environ.has_key('METHOD'):
        self.options.method = os.environ['METHOD']
      else:
        self.options.method = 'opt'

    # Update libmesh_dir to reflect arguments
    if opts.libmesh_dir != '':
      self.libmesh_dir = opts.libmesh_dir

  def postRun(self, test, timing):
    return

  def preRun(self):
    return

  def populateParams(self, params, test):
    # TODO: Print errors or warnings about unused parameters
    # Set difference

    # viewkeys does not work with older Python...
#    unused_params = test.viewkeys() - params.desc
    params.valid.update(test)

    return params


  #### Factory Methods
  def registerTester(self, type, name):
    self.testers[name] = type

  def getValidParams(self, type):
    return self.testers[type].getValidParams()

  def create(self, type, specs):
    return self.testers[type]('exodiff', specs)
  ###

# Notes:
# SHOULD_CRASH returns > 0, cuz < 0 means process interrupted
