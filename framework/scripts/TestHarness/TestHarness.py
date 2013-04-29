import os, sys, re, inspect, types, errno, pprint
import ParseGetPot
from socket import gethostname
from options import *
from util import *
from time import sleep
from RunParallel import RunParallel
from CSVDiffer import CSVDiffer
from Tester import Tester
from InputParameters import InputParameters
from Factory import Factory

from optparse import OptionParser, OptionGroup, Values
#from optparse import OptionG
from timeit import default_timer as clock

class TestHarness:
  def __init__(self, argv, app_name, moose_dir):
    self.factory = Factory()
    self.input_file_name = 'tests'  # This is the file we look for, when looking for test specifications.
    self.test_table = []
    self.num_passed = 0
    self.num_failed = 0
    self.num_skipped = 0
    self.host_name = gethostname()
    self.moose_dir = os.path.abspath(moose_dir) + '/'
    self.code = '2d2d6769726c2d6d6f6465'
    self.MAX_VALGRIND_FAILS = 5
    # Assume libmesh is a peer directory to MOOSE if not defined
    if os.environ.has_key("LIBMESH_DIR"):
      self.libmesh_dir = os.environ['LIBMESH_DIR']
    else:
      self.libmesh_dir = self.moose_dir + '../libmesh/installed'
    self.file = None

    # Parse arguments
    self.parseCLArgs(argv)

    self.checks = {}
    self.checks[PLATFORM] = getPlatforms()
    self.checks[COMPILER] = getCompilers(self.libmesh_dir)
    self.checks[PETSC_VERSION] = getPetscVersion(self.libmesh_dir)
    self.checks[MESH_MODE] = getLibMeshConfigOption(self.libmesh_dir, MESH_MODE)
    self.checks[DTK] =  getLibMeshConfigOption(self.libmesh_dir, DTK)
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
              # Build the requested Tester object and run
              tester = self.factory.create(test[TYPE], test)

              # When running in valgrind mode, we end up with a ton of output for each failed
              # test.  Therefore, we limit the number of fails...
              if self.options.enable_valgrind and self.num_failed > self.MAX_VALGRIND_FAILS:
                (should_run, reason) = (False, 'Max Fails Exceeded')
              else:
                (should_run, reason) = tester.checkRunnableBase(self.options, self.checks)

              if should_run:
                command = tester.getCommand(self.options)

                # This method spawns another process and allows this loop to continue looking for tests
                # RunParallel will call self.testOutputAndFinish when the test has completed running
                # This method will block when the maximum allowed parallel processes are running
                self.runner.run(tester, command)
              else: # This job is skipped - notify the runner
                if (reason != ''):
                  self.handleTestResult(test, '', reason)
                self.runner.jobSkipped(test[TEST_NAME])

            os.chdir(saved_cwd)
            sys.path.pop()

    # Wait for all tests to finish
    self.runner.join()
    self.cleanupAndExit()

  def parseGetPotTestFormat(self, filename):
    tests = []
    test_dir = os.path.abspath(os.path.dirname(filename))
    # Get relative path to test[TEST_DIR]
    executable_path = os.path.dirname(self.executable)
    relative_path = test_dir.replace(executable_path, '')

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
        # First retrieve the type so we can get the valid params
        if TYPE not in test_node.params:
          print "Type missing in " + test_dir + filename
          sys.exit(1)

        params = self.factory.getValidParams(test_node.params[TYPE])

        # Now update all the base level keys
        params_parsed = set()
        params_ignored = set()
        for key, value in test_node.params.iteritems():
          params_parsed.add(key)
          if key in params:
            if params.type(key) == list:
              params[key] = value.split(' ')
            else:
	      if re.match('".*"', value):  # Strip quotes
	        params[key] = value[1:-1]
	      else:
                params[key] = value
          else:
            params_ignored.add(key)

        # Make sure that all required parameters are supplied
        required_params_missing = params.required_keys() - params_parsed
        if len(required_params_missing):
          print 'Required Missing Parameter(s): ', required_params_missing
        if len(params_ignored):
          print 'Ignored Parameter(s): ', params_ignored

	# TODO: In progress formatting
#        formatted_name = relative_path + '.' + testname
        formatted_name = relative_path.replace('/tests/', '') + '.' + testname
#	formatted_name = '/'.join(relative_path.split('/')[1:]) + '.' + testname
#        formatted_name += ' (' + relative_path
#        if test[INPUT] != 'input':  # See if a testname was provided
#          formatted_name += '/' + test[INPUT]
#        formatted_name += ')'

        params[TEST_NAME] = formatted_name
        params[TEST_DIR] = test_dir
        params[RELATIVE_PATH] = relative_path
        params[EXECUTABLE] = self.executable
        params[HOSTNAME] = self.host_name
	params[MOOSE_DIR] = self.moose_dir
        if params.isValid(PREREQ):
          if type(params[PREREQ]) != list:
            print "Option 'PREREQ' needs to be of type list in " + params[TEST_NAME]
            sys.exit(1)
          params[PREREQ] = [relative_path.replace('/tests/', '') + '.' + item for item in params[PREREQ]]

        # Build a list of test specs (dicts) to return
        tests.append(params)
    return tests

  def parseLegacyTestFormat(self, filename):
    print colorText("WARNING: The python test specification format has been deprecated.  Please migrate your test files to the new getpot format", self.options, "RED")

    # dynamically load the module
    module_name = filename[:-3]   # Always a python file (*.py)
    module = __import__(module_name)
    test_dir = os.path.dirname(module.__file__)
    tests = []

    for test_name, test_opts in inspect.getmembers(module):
      if isinstance(test_opts, types.DictType) and self.test_match.search(test_name):

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
	  test[SHOULD_CRASH] = True

        test.update( { TEST_NAME : testname, TEST_DIR : test_dir, RELATIVE_PATH : relative_path, EXECUTABLE : self.executable, HOSTNAME : self.host_name, MOOSE_DIR : self.moose_dir} )

	if TYPE not in test:
	  test.update( { TYPE : "Exodiff" } )

        if test[PREREQ] != None:
          if type(test[PREREQ]) != list:
            print "Option 'PREREQ' needs to be of type list in " + module_name + '.' + test[TEST_NAME]
            sys.exit(1)
          test[PREREQ] = [module_name + '.' + item for item in test[PREREQ]]

	# Create an InputParmaters object using the raw test specs dictionary
	params = self.populateParams(InputParameters(), test)

        # Build a list of test specs (dicts) to return
        tests.append(params)
    return tests

  def augmentTestSpecs(self, test):
    test[EXECUTABLE] = self.executable
    test[HOSTNAME] = self.host_name

  ## Finish the test by inspecting the raw output
  def testOutputAndFinish(self, tester, retcode, output, start=0, end=0):
    caveats = []
    test = tester.specs  # Need to refactor

    if test.isValid('CAVEATS'):
      caveats = test['CAVEATS']

    (reason, output) = tester.processResults(self.moose_dir, retcode, self.options, output)

    if self.options.scaling and test[SCALE_REFINE]:
      caveats.append('SCALED')

    did_pass = True
    if reason == '':
      # It ran OK but is this test set to be skipped on any platform, compiler, so other reason?
      if self.options.extra_info:
        checks = [PLATFORM, COMPILER, PETSC_VERSION, MESH_MODE, METHOD, LIBRARY_MODE, DTK]
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

  def getSolveTime(self, output):
    time = ''
    m = re.search(r"solve().*", output)
    if m != None:
      return m.group().split()[5]

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
    elif self.options.store_time:
      timing = self.getSolveTime(output)

    self.test_table.append( (specs, output, result, timing, start, end) )
    self.postRun(specs, timing)

    if self.options.show_directory:
      print printResult(specs[RELATIVE_PATH] + '/' + specs[TEST_NAME].split('/')[-1], result, timing, start, end, self.options)
    else:
      print printResult(specs[TEST_NAME], result, timing, start, end, self.options)

    if result.find('OK') != -1:
      self.num_passed += 1
    elif result.find('skipped') != -1:
      self.num_skipped += 1
    else:
      self.num_failed += 1

    if self.options.verbose or ('FAILED' in result and not self.options.quiet):
      lines = output.split('\n');
      color = ''
      if 'EXODIFF' in result or 'CSVDIFF' in result:
        color = 'YELLOW'
      elif 'FAILED' in result:
        color = 'RED'
      else:
        color = 'GREEN'
      test_name = colorText(specs[TEST_NAME]  + ": ", self.options, color)
      output = ("\n" + test_name).join(lines)
      print output

      # Print result line again at the bottom of the output for failed tests
      if self.options.show_directory:
        print printResult(specs[RELATIVE_PATH] + '/' + specs[TEST_NAME].split('/')[-1], result, timing, start, end, self.options), "(reprint)"
      else:
        print printResult(specs[TEST_NAME], result, timing, start, end, self.options), "(reprint)"


    if not 'skipped' in result:
      if self.options.file:
        if self.options.show_directory:
          self.file.write(printResult( specs[RELATIVE_PATH] + '/' + specs[TEST_NAME].split('/')[-1], result, timing, start, end, self.options, color=False) + '\n')
          self.file.write(output)
        else:
          self.file.write(printResult( specs[TEST_NAME], result, timing, start, end, self.options, color=False) + '\n')
          self.file.write(output)

      if self.options.sep_files or (self.options.fail_files and 'FAILED' in result) or (self.options.ok_files and result.find('OK') != -1):
        fname = os.path.join(specs[TEST_DIR], specs[TEST_NAME].split('/')[-1] + '.' + result[:6] + '.txt')
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
          print printResult(test[RELATIVE_PATH] + '/' + specs[TEST_NAME].split('/')[-1], result, timing, start, end, self.options)
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
    print colorText( summary % (self.num_passed, self.num_skipped, self.num_failed), self.options, "", html=True )

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
    parser.add_option('--devel', action='store_const', dest='method', const='dev', help='test the app_name-devel binary')
    parser.add_option('--oprof', action='store_const', dest='method', const='oprof', help='test the app_name-oprof binary')
    parser.add_option('--pro', action='store_const', dest='method', const='pro', help='test the app_name-pro binary')
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
    outputgroup.add_option("--yaml", action="store_true", dest="yaml", default=False, help="Dump the parameters for the testers in Yaml Format")
    outputgroup.add_option("--dump", action="store_true", dest="dump", default=False, help="Dump the parameters for the testers in GetPot Format")

    parser.add_option_group(outputgroup)
    code = True
    if self.code.decode('hex') in argv:
      del argv[argv.index(self.code.decode('hex'))]
      code = False
    (self.options, self.tests) = parser.parse_args(argv[1:])
    self.options.ensure_value('code', code)
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
    if opts.store_time:
      # timing returns Active Time, while store_timing returns Solve Time.
      # Thus we need to turn off timing.
      opts.timing = False
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

  def postRun(self, specs, timing):
    return

  def preRun(self):
    if self.options.yaml:
      self.printYaml()
    elif self.options.dump:
      self.printDump()

  def populateParams(self, params, test):
    # TODO: Print errors or warnings about unused parameters
    # Set difference

    # viewkeys does not work with older Python...
#    unused_params = test.viewkeys() - params.desc
    params.valid = test
    return params

  def registerTester(self, type, name):
    self.factory.register(type, name)

  ### Parameter Dump ###
  def printDump(self):
    self.factory.printDump("Tests")
    sys.exit(0)

  def printYaml(self):
    print "**START YAML DATA**"
    print "- name: /Tests"
    print "  description: !!str"
    print "  type:"
    print "  parameters:"
    print "  subblocks:"

    for name, tester in self.testers.iteritems():
      print "  - name: /Tests/" + name
      print "    description:"
      print "    type:"
      print "    parameters:"

      params = self.factory.getValidParams(name)
      for key in params.valid:
        required = 'No'
        if params.isRequired(key):
          required = 'Yes'
        default = ''
        if params.isValid(key):
          default = str(params[key])

        print "    - name: " + key
        print "      required: " + required
        print "      default: !!str " + default
        print "      description: |"
        print "        " + params.getDescription(key)

    print "**END YAML DATA**"
    sys.exit(0)

# Notes:
# SHOULD_CRASH returns > 0, cuz < 0 means process interrupted
