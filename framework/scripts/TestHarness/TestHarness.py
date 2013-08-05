import os, sys, re, inspect, types, errno, pprint, subprocess, io, shutil
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

import argparse
from optparse import OptionParser, OptionGroup, Values
from timeit import default_timer as clock

class TestHarness:
  def __init__(self, argv, app_name, moose_dir):
    self.factory = Factory()

    self.test_table = []
    self.num_passed = 0
    self.num_failed = 0
    self.num_skipped = 0
    self.num_pending = 0
    self.host_name = gethostname()
    self.moose_dir = os.path.abspath(moose_dir) + '/'
    self.code = '2d2d6769726c2d6d6f6465'
    self.processingPBS = False
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

    # PBS STUFF
    if self.options.pbs and os.path.exists(self.options.pbs):
      self.processingPBS = True
      self.processPBSResults()
    else:
      for dirpath, dirnames, filenames in os.walk(os.getcwd(), followlinks=True):
        if (self.test_match.search(dirpath)):
          for file in filenames:
            # set cluster_handle to be None initially (happens for each test)
            self.options.cluster_handle = None
            # See if there were other arguments (test names) passed on the command line
            if file == self.options.input_file_name or file[-2:] == 'py' and self.test_match.search(file):
              saved_cwd = os.getcwd()
              sys.path.append(os.path.abspath(dirpath))
              os.chdir(dirpath)
              if file == self.options.input_file_name:  # New GetPot file formatted test
                tests = self.parseGetPotTestFormat(file)
              elif file[-2:] == 'py' and self.test_match.search(file): # Legacy file formatted test
                tests = self.parseLegacyTestFormat(file)
              # Go through the list of test specs and run them
              for test in tests:
                # Strip begining and ending spaces to input file name
                test[INPUT] = test[INPUT].strip()

                # Build the requested Tester object and run
                tester = self.factory.create(test[TYPE], test)

                # When running in valgrind mode, we end up with a ton of output for each failed
                # test.  Therefore, we limit the number of fails...
                if self.options.enable_valgrind and self.num_failed > self.options.valgrind_max_fails:
                  (should_run, reason) = (False, 'Max Fails Exceeded')
                else:
                  (should_run, reason) = tester.checkRunnableBase(self.options, self.checks)

                if should_run:
                  # Create the cluster launcher input file
                  if self.options.pbs and self.options.cluster_handle == None:
                    self.options.cluster_handle = open(dirpath + '/tests.cluster', 'a')
                    self.options.cluster_handle.write('[Jobs]\n')

                  command = tester.getCommand(self.options)
                  # This method spawns another process and allows this loop to continue looking for tests
                  # RunParallel will call self.testOutputAndFinish when the test has completed running
                  # This method will block when the maximum allowed parallel processes are running
                  self.runner.run(tester, command)
                else: # This job is skipped - notify the runner
                  if (reason != ''):
                    self.handleTestResult(test, '', reason)
                  self.runner.jobSkipped(test[TEST_NAME])

                if self.options.cluster_handle != None:
                  self.options.cluster_handle.write('[]\n')
                  self.options.cluster_handle.close()
                  self.options.cluster_handle = None

              os.chdir(saved_cwd)
              sys.path.pop()

    self.runner.join()
    # Wait for all tests to finish
    if self.options.pbs and self.processingPBS == False:
      print '\n< checking batch status >\n'
      self.processingPBS = True
      self.processPBSResults()
      self.cleanupAndExit()
    else:
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
    except:        # ParseGetPot class
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
    if len(tests) > 0:
      print colorText("WARNING: The python test specification format has been deprecated.  Please migrate your test files to the new getpot format", self.options, "RED")
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

    if self.options.pbs and self.processingPBS == False:
      (reason, output) = self.buildPBSBatch(output, tester)
    else:
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
      elif self.options.pbs and self.processingPBS == False:
        result = 'LAUNCHED'
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

# PBS Defs
  def processPBSResults(self):
    # If batch file exists, check the contents for pending tests.
    if os.path.exists(self.options.pbs):
      # Build a list of launched jobs
      batch_file = open(self.options.pbs)
      batch_list = [y.split(':') for y in [x for x in batch_file.read().split('\n')]]
      batch_file.close()
      del batch_list[-1:]

      # Loop through launched jobs and match the TEST_NAME to determin correct stdout (Output_Path)
      for job in batch_list:
        file = '/'.join(job[2].split('/')[:-2]) + '/' + job[3]
        tests = self.parseGetPotTestFormat(file)
        for test in tests:
          # Build the requested Tester object
          if job[1] == test[TEST_NAME]:
            # Create Test Type
            tester = self.factory.create(test[TYPE], test)

            # Get job status via qstat
            qstat = ['qstat', '-f', '-x', str(job[0])]
            qstat_command = subprocess.Popen(qstat, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            qstat_stdout = qstat_command.communicate()[0]
            if qstat_stdout != None:
              output_value = re.search(r'job_state = (\w+)', qstat_stdout).group(1)
            else:
              return ('QSTAT NOT FOUND', '')

            # Report the current status of JOB_ID
            if output_value == 'F':
              # If the job is finished, analyze the results
              if os.path.exists(job[2]):
                output_file = open(job[2], 'r')
                # Not sure I am doing this right: I have to change the TEST_DIR to match the temporary cluster_launcher TEST_DIR location, thus violating the tester.specs...
                test[TEST_DIR] = '/'.join(job[2].split('/')[:-1])
                self.testOutputAndFinish(tester, 0, output_file.read())
                output_file.close()
              else:
                # I ran into this scenario when the cluster went down, but launched/completed my job :)
                self.handleTestResult(tester.specs, '', 'FAILED (NO STDOUT FILE)', 0, 0)
            elif output_value == 'R':
              # Job is currently running
              self.handleTestResult(tester.specs, '', 'RUNNING', 0, 0)
            elif output_value == 'E':
              # Job is exiting
              self.handleTestResult(tester.specs, '', 'EXITING', 0, 0)
            elif output_value == 'Q':
              # Job is currently queued
              self.handleTestResult(tester.specs, '', 'QUEUED', 0, 0)
    else:
      return ('BATCH FILE NOT FOUND', '')

  def buildPBSBatch(self, output, tester):
    # Create/Update the batch file
    if 'command not found' in output:
      return('QSUB NOT FOUND', '')
    else:
      # Get the PBS Job ID using qstat
      # TODO: Build an error handler. If there was any issue launching the cluster launcher due to <any thing>, why die here.
      job_id = re.findall(r'.*JOB_ID: (\d+)', output)[0]
      qstat = ['qstat', '-f', '-x', str(job_id)]
      qstat_command = subprocess.Popen(qstat, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
      qstat_stdout = qstat_command.communicate()[0]

      # Get the Output_Path from qstat stdout
      if qstat_stdout != None:
        output_value = re.search(r'Output_Path(.*?)(^ +)', qstat_stdout, re.S | re.M).group(1)
        output_value = output_value.split(':')[1].replace('\n', '').replace('\t', '')
      else:
        return ('QSTAT NOT FOUND', '')

      # Write job_id, test[TEST_NAME], and Ouput_Path to the batch file
      file_name = self.options.pbs
      job_list = open(os.path.abspath(os.path.join(tester.specs[EXECUTABLE], os.pardir)) + '/' + file_name, 'a')
      job_list.write(str(job_id) + ':' + tester.specs[TEST_NAME] + ':' + output_value + ':' + self.options.input_file_name  + '\n')
      job_list.close()

      # Return to TestHarness and inform we have launched the job
      return ('', 'LAUNCHED')

  def cleanPBSBatch(self):
    # Open the PBS batch file and assign it to a list
    if os.path.exists(self.options.pbs_cleanup):
      batch_file = open(self.options.pbs_cleanup, 'r')
      batch_list = [y.split(':') for y in [x for x in batch_file.read().split('\n')]]
      batch_file.close()
      del batch_list[-1:]
    else:
      print 'PBS batch file not found:', self.options.pbs_cleanup
      sys.exit(1)

    # Loop through launched jobs and delete whats found.
    for job in batch_list:
      if os.path.exists(job[2]):
        batch_dir = os.path.abspath(os.path.join(job[2], os.pardir)).split('/')
        if os.path.exists('/'.join(batch_dir)):
          shutil.rmtree('/'.join(batch_dir))
        if os.path.exists('/'.join(batch_dir[:-1]) + '/' + job[3] + '.cluster'):
          os.remove('/'.join(batch_dir[:-1]) + '/' + job[3] + '.cluster')
    os.remove(self.options.pbs_cleanup)

# END PBS Defs

  ## Update global variables and print output based on the test result
  # Containing OK means it passed, skipped means skipped, anything else means it failed
  def handleTestResult(self, specs, output, result, start=0, end=0):
    timing = ''

    if self.options.timing:
      timing = self.getTiming(output)
    elif self.options.store_time:
      timing = self.getSolveTime(output)

    # I have to evaluate processingPBS like this, because there are two stages of processingPBS (launching and then evaluating output)...
    if self.processingPBS:
      self.test_table.append( (specs, output, result, timing, start, end) )
    # Normal operation
    elif self.options.pbs == None:
      self.test_table.append( (specs, output, result, timing, start, end) )

    self.postRun(specs, timing)

    if self.options.show_directory:
      print printResult(specs[RELATIVE_PATH] + '/' + specs[TEST_NAME].split('/')[-1], result, timing, start, end, self.options)
    else:
      print printResult(specs[TEST_NAME], result, timing, start, end, self.options)

    if self.processingPBS:
      if result.find('OK') != -1:
        self.num_passed += 1
      elif result.find('skipped') != -1:
        self.num_skipped += 1
      elif result.find('deleted') != -1:
        self.num_skipped += 1
      elif result.find('LAUNCHED') != -1 or result.find('RUNNING') != -1 or result.find('QUEUED') != -1 or result.find('EXITING') != -1:
        self.num_pending += 1
      else:
        self.num_failed += 1
    elif self.options.pbs == None:
      if result.find('OK') != -1:
        self.num_passed += 1
      elif result.find('skipped') != -1:
        self.num_skipped += 1
      elif result.find('deleted') != -1:
        self.num_skipped += 1
      elif result.find('LAUNCHED') != -1 or result.find('RUNNING') != -1 or result.find('QUEUED') != -1 or result.find('EXITING') != -1:
        self.num_pending += 1
      else:
        self.num_failed += 1

    if self.options.verbose or ('FAILED' in result and not self.options.quiet):
      output = output.replace('\r', '\n')  # replace the carriage returns with newlines
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
    summary += ', <b>%d skipped</b>'
    if self.num_pending:
      summary += ', <c>%d pending</c>, '
    else:
      summary += ', <b>%d pending</b>, '
    if self.num_failed:
      summary += '<r>%d FAILED</r>'
    else:
      summary += '<b>%d failed</b>'

    print colorText( summary % (self.num_passed, self.num_skipped, self.num_pending, self.num_failed), self.options, "", html=True )
    if self.options.pbs:
      print '\nYour PBS batch file:', self.options.pbs
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
  def parseCLArgs(self, argv=sys.argv[1:]):
    parser = argparse.ArgumentParser(description='A tool used to test MOOSE based applications')
    parser.add_argument('test_name', nargs=argparse.REMAINDER)
    parser.add_argument('--opt', action='store_const', dest='method', const='opt', help='test the app_name-opt binary')
    parser.add_argument('--dbg', action='store_const', dest='method', const='dbg', help='test the app_name-dbg binary')
    parser.add_argument('--devel', action='store_const', dest='method', const='dev', help='test the app_name-devel binary')
    parser.add_argument('--oprof', action='store_const', dest='method', const='oprof', help='test the app_name-oprof binary')
    parser.add_argument('--pro', action='store_const', dest='method', const='pro', help='test the app_name-pro binary')
    parser.add_argument('-j', '--jobs', nargs=1, metavar='int', action='store', type=int, dest='jobs', default=1, help='run test binaries in parallel')
    parser.add_argument('-e', action='store_true', dest='extra_info', help='Display "extra" information including all caveats and deleted tests')
    parser.add_argument('-c', '--no-color', action='store_false', dest='colored', help='Do not show colored output')
    parser.add_argument('--heavy', action='store_true', dest='heavy_tests', help='Run tests marked with HEAVY : True')
    parser.add_argument('--all-tests', action='store_true', dest='all_tests', help='Run normal tests and tests marked with HEAVY : True')
    parser.add_argument('-g', '--group', action='store', type=str, dest='group', default='ALL', help='Run only tests in the named group')
    parser.add_argument('--not_group', action='store', type=str, dest='not_group', help='Run only tests NOT in the named group')
#    parser.add_argument('--dofs', action='store', dest='dofs', help='This option is for automatic scaling which is not currently implemented in MOOSE 2.0')
    parser.add_argument('--dbfile', nargs='?', action='store', dest='dbFile', help='Location to timings data base file. If not set, assumes $HOME/timingDB/timing.sqlite')
    parser.add_argument('-l', '--load-average', action='store', type=float, dest='load', default=64.0, help='Do not run additional tests if the load average is at least LOAD')
    parser.add_argument('-t', '--timing', action='store_true', dest='timing', help='Report Timing information for passing tests')
    parser.add_argument('-s', '--scale', action='store_true', dest='scaling', help='Scale problems that have SCALE_REFINE set')
    parser.add_argument('-i', nargs=1, action='store', type=str, dest='input_file_name', default='tests', help='The default test specification file to look for (default="tests").')
    parser.add_argument('--libmesh_dir', nargs=1, action='store', type=str, dest='libmesh_dir', help='Currently only needed for bitten code coverage')
    parser.add_argument('--parallel', '-p', nargs='?', action='store', type=int, dest='parallel', const=1, help='Number of processors to use when running mpiexec')
    parser.add_argument('--n-threads', nargs=1, action='store', type=int, dest='nthreads', default=1, help='Number of threads to use when running mpiexec')
    parser.add_argument('-d', action='store_true', dest='debug_harness', help='Turn on Test Harness debugging')
    parser.add_argument('--valgrind', action='store_true', dest='enable_valgrind', help='Enable Valgrind')
    parser.add_argument('--valgrind-max-fails', nargs=1, type=int, dest='valgrind_max_fails', default=5, help='The number of valgrind tests allowed to fail before any additional valgrind tests will run')
    parser.add_argument('--pbs', nargs='?', metavar='batch_file', dest='pbs', const='generate', help='Enable launching tests via PBS. If no batch file is specified one will be created for you')
    parser.add_argument('--pbs-cleanup', nargs=1, metavar='batch_file', help='Clean up the directories/files created by PBS. You must supply the same batch_file used to launch PBS.')
    parser.add_argument('--re', action='store', type=str, dest='reg_exp', help='Run tests that match --re=regular_expression')
    parser.add_argument('--cli-args', nargs='?', type=str, dest='cli_args', help='Append the following list of agruments to the command line (Encapsulate the command in quotes)')

    outputgroup = parser.add_argument_group('Output Options', 'These options control the output of the test harness. The sep-files options write output to files named test_name.TEST_RESULT.txt. All file output will overwrite old files')
    outputgroup.add_argument('-v', '--verbose', action='store_true', dest='verbose', help='show the output of every test that fails')
    outputgroup.add_argument('-q', '--quiet', action='store_true', dest='quiet', help='only show the result of every test, don\'t show test output even if it fails')
    outputgroup.add_argument('--show-directory', action='store_true', dest='show_directory', help='Print test directory path in out messages')
    outputgroup.add_argument('-o', '--output-dir', nargs=1, metavar='directory', dest='output_dir', default='', help='Save all output files in the directory, and create it if necessary')
    outputgroup.add_argument('-f', '--file', nargs=1, action='store', dest='file', help='Write verbose output of each test to FILE and quiet output to terminal')
    outputgroup.add_argument('-x', '--sep-files', action='store_true', dest='sep_files', help='Write the output of each test to a separate file. Only quiet output to terminal. This is equivalant to \'--sep-files-fail --sep-files-ok\'')
    outputgroup.add_argument('--sep-files-ok', action='store_true', dest='ok_files', help='Write the output of each passed test to a separate file')
    outputgroup.add_argument('-a', '--sep-files-fail', action='store_true', dest='fail_files', help='Write the output of each FAILED test to a separate file. Only quiet output to terminal.')
    outputgroup.add_argument("--store-timing", action="store_true", dest="store_time", help="Store timing in the SQL database: $HOME/timingDB/timing.sqlite A parent directory (timingDB) must exist.")
    outputgroup.add_argument("--revision", nargs=1, action="store", type=int, dest="revision", help="The current revision being tested. Required when using --store-timing.")
    outputgroup.add_argument("--yaml", action="store_true", dest="yaml", help="Dump the parameters for the testers in Yaml Format")
    outputgroup.add_argument("--dump", action="store_true", dest="dump", help="Dump the parameters for the testers in GetPot Format")

    code = True
    if self.code.decode('hex') in argv:
      del argv[argv.index(self.code.decode('hex'))]
      code = False
    self.options = parser.parse_args()
    self.tests = self.options.test_name
    self.options.code = code

    # Convert all list based options of length one to scalars
    for key, value in vars(self.options).items():
      if type(value) == list and len(value) == 1:
        tmp_str = getattr(self.options, key)
        setattr(self.options, key, value[0])

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
    if opts.libmesh_dir:
      self.libmesh_dir = opts.libmesh_dir

    # Generate a batch file if PBS argument supplied with out a file
    if opts.pbs == 'generate':
      largest_serial_num = 0
      for name in os.listdir('.'):
        m = re.search('pbs_(\d{3})', name)
        if m != None and int(m.group(1)) > largest_serial_num:
          largest_serial_num = int(m.group(1))
      opts.pbs = "pbs_" +  str(largest_serial_num+1).zfill(3)

  def postRun(self, specs, timing):
    return

  def preRun(self):
    if self.options.yaml:
      self.factory.printYaml("Tests")
      sys.exit(0)
    elif self.options.dump:
      self.factory.printDump("Tests")
      sys.exit(0)
    if self.options.pbs_cleanup:
      self.cleanPBSBatch()
      sys.exit(0)

  def populateParams(self, params, test):
    # TODO: Print errors or warnings about unused parameters
    # Set difference

    # viewkeys does not work with older Python...
#    unused_params = test.viewkeys() - params.desc
    params.valid = test
    return params

  def getFactory(self):
    return self.factory

# Notes:
# SHOULD_CRASH returns > 0, cuz < 0 means process interrupted
