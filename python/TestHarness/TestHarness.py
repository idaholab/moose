import os, sys, re, inspect, types, errno, pprint, subprocess, io, shutil, time, copy
import path_tool

path_tool.activate_module('FactorySystem')
path_tool.activate_module('argparse')

from ParseGetPot import ParseGetPot
from socket import gethostname
#from options import *
from util import *
from RunParallel import RunParallel
from CSVDiffer import CSVDiffer
from XMLDiffer import XMLDiffer
from Tester import Tester
from PetscJacobianTester import PetscJacobianTester
from InputParameters import InputParameters
from Factory import Factory
from Parser import Parser
from Warehouse import Warehouse

import argparse
from optparse import OptionParser, OptionGroup, Values
from timeit import default_timer as clock

class TestHarness:

  @staticmethod
  def buildAndRun(argv, app_name, moose_dir):
    if '--store-timing' in argv:
      harness = TestTimer(argv, app_name, moose_dir)
    else:
      harness = TestHarness(argv, app_name, moose_dir)

    harness.findAndRunTests()

    sys.exit(harness.error_code)


  def __init__(self, argv, app_name, moose_dir):
    self.factory = Factory()

    # Build a Warehouse to hold the MooseObjects
    self.warehouse = Warehouse()

    # Get dependant applications and load dynamic tester plugins
    # If applications have new testers, we expect to find them in <app_dir>/scripts/TestHarness/testers
    dirs = [os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))]
    sys.path.append(os.path.join(moose_dir, 'framework', 'scripts'))   # For find_dep_apps.py

    # Use the find_dep_apps script to get the dependant applications for an app
    import find_dep_apps
    depend_app_dirs = find_dep_apps.findDepApps(app_name)
    dirs.extend([os.path.join(my_dir, 'scripts', 'TestHarness') for my_dir in depend_app_dirs.split('\n')])

    # Finally load the plugins!
    self.factory.loadPlugins(dirs, 'testers', Tester)

    self.test_table = []
    self.num_passed = 0
    self.num_failed = 0
    self.num_skipped = 0
    self.num_pending = 0
    self.host_name = gethostname()
    self.moose_dir = moose_dir
    self.base_dir = os.getcwd()
    self.run_tests_dir = os.path.abspath('.')
    self.code = '2d2d6769726c2d6d6f6465'
    self.error_code = 0x0
    # Assume libmesh is a peer directory to MOOSE if not defined
    if os.environ.has_key("LIBMESH_DIR"):
      self.libmesh_dir = os.environ['LIBMESH_DIR']
    else:
      self.libmesh_dir = os.path.join(self.moose_dir, 'libmesh', 'installed')
    self.file = None

    # Parse arguments
    self.parseCLArgs(argv)

    self.checks = {}
    self.checks['platform'] = getPlatforms()
    self.checks['submodules'] = getInitializedSubmodules(self.run_tests_dir)

    # The TestHarness doesn't strictly require the existence of libMesh in order to run. Here we allow the user
    # to select whether they want to probe for libMesh configuration options.
    if self.options.skip_config_checks:
      self.checks['compiler'] = set(['ALL'])
      self.checks['petsc_version'] = 'N/A'
      self.checks['library_mode'] = set(['ALL'])
      self.checks['mesh_mode'] = set(['ALL'])
      self.checks['dtk'] = set(['ALL'])
      self.checks['unique_ids'] = set(['ALL'])
      self.checks['vtk'] = set(['ALL'])
      self.checks['tecplot'] = set(['ALL'])
      self.checks['dof_id_bytes'] = set(['ALL'])
      self.checks['petsc_debug'] = set(['ALL'])
      self.checks['curl'] = set(['ALL'])
      self.checks['tbb'] = set(['ALL'])
      self.checks['superlu'] = set(['ALL'])
      self.checks['slepc'] = set(['ALL'])
      self.checks['unique_id'] = set(['ALL'])
      self.checks['cxx11'] = set(['ALL'])
      self.checks['asio'] =  set(['ALL'])
    else:
      self.checks['compiler'] = getCompilers(self.libmesh_dir)
      self.checks['petsc_version'] = getPetscVersion(self.libmesh_dir)
      self.checks['library_mode'] = getSharedOption(self.libmesh_dir)
      self.checks['mesh_mode'] = getLibMeshConfigOption(self.libmesh_dir, 'mesh_mode')
      self.checks['dtk'] =  getLibMeshConfigOption(self.libmesh_dir, 'dtk')
      self.checks['unique_ids'] = getLibMeshConfigOption(self.libmesh_dir, 'unique_ids')
      self.checks['vtk'] =  getLibMeshConfigOption(self.libmesh_dir, 'vtk')
      self.checks['tecplot'] =  getLibMeshConfigOption(self.libmesh_dir, 'tecplot')
      self.checks['dof_id_bytes'] = getLibMeshConfigOption(self.libmesh_dir, 'dof_id_bytes')
      self.checks['petsc_debug'] = getLibMeshConfigOption(self.libmesh_dir, 'petsc_debug')
      self.checks['curl'] =  getLibMeshConfigOption(self.libmesh_dir, 'curl')
      self.checks['tbb'] =  getLibMeshConfigOption(self.libmesh_dir, 'tbb')
      self.checks['superlu'] =  getLibMeshConfigOption(self.libmesh_dir, 'superlu')
      self.checks['slepc'] =  getLibMeshConfigOption(self.libmesh_dir, 'slepc')
      self.checks['unique_id'] =  getLibMeshConfigOption(self.libmesh_dir, 'unique_id')
      self.checks['cxx11'] =  getLibMeshConfigOption(self.libmesh_dir, 'cxx11')
      self.checks['asio'] =  getIfAsioExists(self.moose_dir)

    # Override the MESH_MODE option if using the '--distributed-mesh'
    # or (deprecated) '--parallel-mesh' option.
    if (self.options.parallel_mesh == True or self.options.distributed_mesh == True) or \
          (self.options.cli_args != None and \
           (self.options.cli_args.find('--parallel-mesh') != -1 or self.options.cli_args.find('--distributed-mesh') != -1)):

      option_set = set(['ALL', 'PARALLEL'])
      self.checks['mesh_mode'] = option_set

    method = set(['ALL', self.options.method.upper()])
    self.checks['method'] = method

    self.initialize(argv, app_name)

  """
  Recursively walks the current tree looking for tests to run
  Error codes:
  0x0  - Success
  0x0* - Parser error
  0x1* - TestHarness error
  """
  def findAndRunTests(self, find_only=False):
    self.error_code = 0x0
    self.preRun()
    self.start_time = clock()

    try:
      # PBS STUFF
      if self.options.pbs:
        # Check to see if we are using the PBS Emulator.
        # Its expensive, so it must remain outside of the os.walk for loop.
        self.options.PBSEmulator = self.checkPBSEmulator()
      if self.options.pbs and os.path.exists(self.options.pbs):
        self.options.processingPBS = True
        self.processPBSResults()
      else:
        self.options.processingPBS = False
        self.base_dir = os.getcwd()
        for dirpath, dirnames, filenames in os.walk(self.base_dir, followlinks=True):
          # Prune submdule paths when searching for tests
          if self.base_dir != dirpath and os.path.exists(os.path.join(dirpath, '.git')):
            dirnames[:] = []

          # walk into directories that aren't contrib directories
          if "contrib" not in os.path.relpath(dirpath, os.getcwd()):
            for file in filenames:
              # set cluster_handle to be None initially (happens for each test)
              self.options.cluster_handle = None
              # See if there were other arguments (test names) passed on the command line
              if file == self.options.input_file_name: #and self.test_match.search(file):
                saved_cwd = os.getcwd()
                sys.path.append(os.path.abspath(dirpath))
                os.chdir(dirpath)

                if self.prunePath(file):
                  continue

                # Build a Parser to parse the objects
                parser = Parser(self.factory, self.warehouse)

                # Parse it
                self.error_code = self.error_code | parser.parse(file)

                # Retrieve the tests from the warehouse
                testers = self.warehouse.getActiveObjects()

                # Augment the Testers with additional information directly from the TestHarness
                for tester in testers:
                  self.augmentParameters(file, tester)

                # Short circuit this loop if we've only been asked to parse Testers
                # Note: The warehouse will accumulate all testers in this mode
                if find_only:
                  self.warehouse.markAllObjectsInactive()
                  continue

                # Clear out the testers, we won't need them to stick around in the warehouse
                self.warehouse.clear()

                if self.options.enable_recover:
                  testers = self.appendRecoverableTests(testers)

                # Handle PBS tests.cluster file
                if self.options.pbs:
                  (tester, command) = self.createClusterLauncher(dirpath, testers)
                  if command is not None:
                    self.runner.run(tester, command)
                else:
                  # Go through the Testers and run them
                  for tester in testers:
                    # Double the alloted time for tests when running with the valgrind option
                    tester.setValgrindMode(self.options.valgrind_mode)

                    # When running in valgrind mode, we end up with a ton of output for each failed
                    # test.  Therefore, we limit the number of fails...
                    if self.options.valgrind_mode and self.num_failed > self.options.valgrind_max_fails:
                      (should_run, reason) = (False, 'Max Fails Exceeded')
                    elif self.num_failed > self.options.max_fails:
                      (should_run, reason) = (False, 'Max Fails Exceeded')
                    elif tester.parameters().isValid('error_code'):
                      (should_run, reason) = (False, 'skipped (Parser Error)')
                    else:
                      (should_run, reason) = tester.checkRunnableBase(self.options, self.checks)

                    if should_run:
                      command = tester.getCommand(self.options)
                      # This method spawns another process and allows this loop to continue looking for tests
                      # RunParallel will call self.testOutputAndFinish when the test has completed running
                      # This method will block when the maximum allowed parallel processes are running
                      self.runner.run(tester, command)
                    else: # This job is skipped - notify the runner
                      if reason != '':
                        if (self.options.report_skipped and reason.find('skipped') != -1) or reason.find('skipped') == -1:
                          self.handleTestResult(tester.parameters(), '', reason)
                      self.runner.jobSkipped(tester.parameters()['test_name'])
                os.chdir(saved_cwd)
                sys.path.pop()
    except KeyboardInterrupt:
      print '\nExiting due to keyboard interrupt...'
      sys.exit(0)

    self.runner.join()
    # Wait for all tests to finish
    if self.options.pbs and self.options.processingPBS == False:
      print '\n< checking batch status >\n'
      self.options.processingPBS = True
      self.processPBSResults()

    self.cleanup()

    # Flags for the parser start at the low bit, flags for the TestHarness start at the high bit
    if self.num_failed:
      self.error_code = self.error_code | 0x80

    return

  def createClusterLauncher(self, dirpath, testers):
    self.options.test_serial_number = 0
    command = None
    tester = None
    # Create the tests.cluster input file
    # Loop through each tester and create a job
    for tester in testers:
      (should_run, reason) = tester.checkRunnableBase(self.options, self.checks)
      if should_run:
        if self.options.cluster_handle == None:
          self.options.cluster_handle = open(dirpath + '/' + self.options.pbs + '.cluster', 'w')
        self.options.cluster_handle.write('[Jobs]\n')
        # This returns the command to run as well as builds the parameters of the test
        # The resulting command once this loop has completed is sufficient to launch
        # all previous jobs
        command = tester.getCommand(self.options)
        self.options.cluster_handle.write('[]\n')
        self.options.test_serial_number += 1
      else: # This job is skipped - notify the runner
        if (reason != ''):
          self.handleTestResult(tester.parameters(), '', reason)
          self.runner.jobSkipped(tester.parameters()['test_name'])

    # Close the tests.cluster file
    if self.options.cluster_handle is not None:
      self.options.cluster_handle.close()
      self.options.cluster_handle = None

    # Return the final tester/command (sufficient to run all tests)
    return (tester, command)


  def prunePath(self, filename):
    test_dir = os.path.abspath(os.path.dirname(filename))

    # Filter tests that we want to run
    # Under the new format, we will filter based on directory not filename since it is fixed
    prune = True
    if len(self.tests) == 0:
      prune = False # No filter
    else:
      for item in self.tests:
        if test_dir.find(item) > -1:
          prune = False

    # Return the inverse of will_run to indicate that this path should be pruned
    return prune

  def augmentParameters(self, filename, tester):
    params = tester.parameters()

    # We are going to do some formatting of the path that is printed
    # Case 1.  If the test directory (normally matches the input_file_name) comes first,
    #          we will simply remove it from the path
    # Case 2.  If the test directory is somewhere in the middle then we should preserve
    #          the leading part of the path
    test_dir = os.path.abspath(os.path.dirname(filename))
    relative_path = test_dir.replace(self.run_tests_dir, '')
    relative_path = relative_path.replace('/' + self.options.input_file_name + '/', ':')
    relative_path = re.sub('^[/:]*', '', relative_path)  # Trim slashes and colons
    formatted_name = relative_path + '.' + tester.name()

    params['test_name'] = formatted_name
    params['test_dir'] = test_dir
    params['relative_path'] = relative_path
    params['executable'] = self.executable
    params['hostname'] = self.host_name
    params['moose_dir'] = self.moose_dir
    params['base_dir'] = self.base_dir

    if params.isValid('prereq'):
      if type(params['prereq']) != list:
        print "Option 'prereq' needs to be of type list in " + params['test_name']
        sys.exit(1)
      params['prereq'] = [relative_path.replace('/tests/', '') + '.' + item for item in params['prereq']]

  # This method splits a lists of tests into two pieces each, the first piece will run the test for
  # approx. half the number of timesteps and will write out a restart file.  The second test will
  # then complete the run using the MOOSE recover option.
  def appendRecoverableTests(self, testers):
    new_tests = []

    for part1 in testers:
      if part1.parameters()['recover'] == True:
        # Clone the test specs
        part2 = copy.deepcopy(part1)

        # Part 1:
        part1_params = part1.parameters()
        part1_params['test_name'] += '_part1'
        part1_params['cli_args'].append('--half-transient Outputs/checkpoint=true')
        part1_params['skip_checks'] = True

        # Part 2:
        part2_params = part2.parameters()
        part2_params['prereq'].append(part1.parameters()['test_name'])
        part2_params['delete_output_before_running'] = False
        part2_params['cli_args'].append('--recover')
        part2_params.addParam('caveats', ['recover'], "")

        new_tests.append(part2)

    testers.extend(new_tests)
    return testers

  ## Finish the test by inspecting the raw output
  def testOutputAndFinish(self, tester, retcode, output, start=0, end=0):
    caveats = []
    test = tester.specs  # Need to refactor

    if test.isValid('caveats'):
      caveats = test['caveats']

    if self.options.pbs and self.options.processingPBS == False:
      (reason, output) = self.buildPBSBatch(output, tester)
    elif self.options.dry_run:
      reason = 'DRY_RUN'
      output += '\n'.join(tester.processResultsCommand(self.moose_dir, self.options))
    else:
      (reason, output) = tester.processResults(self.moose_dir, retcode, self.options, output)

    if self.options.scaling and test['scale_refine']:
      caveats.append('scaled')

    did_pass = True
    if reason == '':
      # It ran OK but is this test set to be skipped on any platform, compiler, so other reason?
      if self.options.extra_info:
        checks = ['platform', 'compiler', 'petsc_version', 'mesh_mode', 'method', 'library_mode', 'dtk', 'unique_ids']
        for check in checks:
          if not 'ALL' in test[check]:
            caveats.append(', '.join(test[check]))
      if len(caveats):
        result = '[' + ', '.join(caveats).upper() + '] OK'
      elif self.options.pbs and self.options.processingPBS == False:
        result = 'LAUNCHED'
      else:
        result = 'OK'
    elif reason == 'DRY_RUN':
      result = 'DRY_RUN'
    else:
      result = 'FAILED (%s)' % reason
      did_pass = False
    if self.options.pbs and self.options.processingPBS == False and did_pass == True:
      # Handle the launch result, but do not add it to the results table (except if we learned that QSUB failed to launch for some reason)
      self.handleTestResult(tester.specs, output, result, start, end, False)
      return did_pass
    else:
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

  def checkPBSEmulator(self):
    try:
      qstat_process = subprocess.Popen(['qstat', '--version'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
      qstat_output = qstat_process.communicate()
    except OSError:
      # qstat binary is not available
      print 'qstat not available. Perhaps you need to load the PBS module?'
      sys.exit(1)
    if len(qstat_output[1]):
      # The PBS Emulator has no --version argument, and thus returns output to stderr
      return True
    else:
      return False

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

        # Build a Warehouse to hold the MooseObjects
        warehouse = Warehouse()

        # Build a Parser to parse the objects
        parser = Parser(self.factory, warehouse)

        # Parse it
        parser.parse(file)

        # Retrieve the tests from the warehouse
        testers = warehouse.getAllObjects()
        for tester in testers:
          self.augmentParameters(file, tester)

        for tester in testers:
          # Build the requested Tester object
          if job[1] == tester.parameters()['test_name']:
            # Create Test Type
            # test = self.factory.create(tester.parameters()['type'], tester)

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
              # F = Finished. Get the exit code reported by qstat
              exit_code = int(re.search(r'Exit_status = (-?\d+)', qstat_stdout).group(1))

              # Read the stdout file
              if os.path.exists(job[2]):
                output_file = open(job[2], 'r')
                # Not sure I am doing this right: I have to change the TEST_DIR to match the temporary cluster_launcher TEST_DIR location, thus violating the tester.specs...
                tester.parameters()['test_dir'] = '/'.join(job[2].split('/')[:-1])
                outfile = output_file.read()
                output_file.close()
                self.testOutputAndFinish(tester, exit_code, outfile)
              else:
                # I ran into this scenario when the cluster went down, but launched/completed my job :)
                self.handleTestResult(tester.specs, '', 'FAILED (NO STDOUT FILE)', 0, 0, True)

            elif output_value == 'R':
              # Job is currently running
              self.handleTestResult(tester.specs, '', 'RUNNING', 0, 0, True)
            elif output_value == 'E':
              # Job is exiting
              self.handleTestResult(tester.specs, '', 'EXITING', 0, 0, True)
            elif output_value == 'Q':
              # Job is currently queued
              self.handleTestResult(tester.specs, '', 'QUEUED', 0, 0, True)
    else:
      return ('BATCH FILE NOT FOUND', '')

  def buildPBSBatch(self, output, tester):
    # Create/Update the batch file
    if 'command not found' in output:
      return ('QSUB NOT FOUND', '')
    else:
      # Get the Job information from the ClusterLauncher
      results = re.findall(r'JOB_NAME: (\w+) JOB_ID:.* (\d+).*TEST_NAME: (\S+)', output)
      if len(results) != 0:
        file_name = self.options.pbs
        job_list = open(os.path.abspath(os.path.join(tester.specs['executable'], os.pardir)) + '/' + file_name, 'a')
        for result in results:
          (test_dir, job_id, test_name) = result
          qstat_command = subprocess.Popen(['qstat', '-f', '-x', str(job_id)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
          qstat_stdout = qstat_command.communicate()[0]
          # Get the Output_Path from qstat stdout
          if qstat_stdout != None:
            output_value = re.search(r'Output_Path(.*?)(^ +)', qstat_stdout, re.S | re.M).group(1)
            output_value = output_value.split(':')[1].replace('\n', '').replace('\t', '').strip()
          else:
            job_list.close()
            return ('QSTAT NOT FOUND', '')
          # Write job_id, test['test_name'], and Ouput_Path to the batch file
          job_list.write(str(job_id) + ':' + test_name + ':' + output_value + ':' + self.options.input_file_name  + '\n')
        # Return to TestHarness and inform we have launched the job
        job_list.close()
        return ('', 'LAUNCHED')
      else:
        return ('QSTAT INVALID RESULTS', output)

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
        if os.path.exists('/'.join(batch_dir[:-1]) + '/' + self.options.pbs_cleanup + '.cluster'):
          os.remove('/'.join(batch_dir[:-1]) + '/' + self.options.pbs_cleanup + '.cluster')
    os.remove(self.options.pbs_cleanup)

# END PBS Defs

  ## Update global variables and print output based on the test result
  # Containing OK means it passed, skipped means skipped, anything else means it failed
  def handleTestResult(self, specs, output, result, start=0, end=0, add_to_table=True):
    timing = ''

    if self.options.timing:
      timing = self.getTiming(output)
    elif self.options.store_time:
      timing = self.getSolveTime(output)

    # Only add to the test_table if told to. We now have enough cases where we wish to print to the screen, but not
    # in the 'Final Test Results' area.
    if add_to_table:
      self.test_table.append( (specs, output, result, timing, start, end) )
      if result.find('OK') != -1 or result.find('DRY_RUN') != -1:
        self.num_passed += 1
      elif result.find('skipped') != -1:
        self.num_skipped += 1
      elif result.find('deleted') != -1:
        self.num_skipped += 1
      elif result.find('LAUNCHED') != -1 or result.find('RUNNING') != -1 or result.find('QUEUED') != -1 or result.find('EXITING') != -1:
        self.num_pending += 1
      else:
        self.num_failed += 1

    self.postRun(specs, timing)

    if self.options.show_directory:
      print printResult(specs['relative_path'] + '/' + specs['test_name'].split('/')[-1], result, timing, start, end, self.options)
    else:
      print printResult(specs['test_name'], result, timing, start, end, self.options)

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
      test_name = colorText(specs['test_name']  + ": ", color, colored=self.options.colored, code=self.options.code)
      output = test_name + ("\n" + test_name).join(lines)
      print output

      # Print result line again at the bottom of the output for failed tests
      if self.options.show_directory:
        print printResult(specs['relative_path'] + '/' + specs['test_name'].split('/')[-1], result, timing, start, end, self.options), "(reprint)"
      else:
        print printResult(specs['test_name'], result, timing, start, end, self.options), "(reprint)"


    if not 'skipped' in result:
      if self.options.file:
        if self.options.show_directory:
          self.file.write(printResult( specs['relative_path'] + '/' + specs['test_name'].split('/')[-1], result, timing, start, end, self.options, color=False) + '\n')
          self.file.write(output)
        else:
          self.file.write(printResult( specs['test_name'], result, timing, start, end, self.options, color=False) + '\n')
          self.file.write(output)

      if self.options.sep_files or (self.options.fail_files and 'FAILED' in result) or (self.options.ok_files and result.find('OK') != -1):
        fname = os.path.join(specs['test_dir'], specs['test_name'].split('/')[-1] + '.' + result[:6] + '.txt')
        f = open(fname, 'w')
        f.write(printResult( specs['test_name'], result, timing, start, end, self.options, color=False) + '\n')
        f.write(output)
        f.close()

  # Write the app_name to a file, if the tests passed
  def writeState(self, app_name):
    # If we encounter bitten_status_moose environment, build a line itemized list of applications which passed their tests
    if os.environ.has_key("BITTEN_STATUS_MOOSE"):
      result_file = open(os.path.join(self.moose_dir, 'test_results.log'), 'a')
      result_file.write(os.path.split(app_name)[1].split('-')[0] + '\n')
      result_file.close()

  # Print final results, close open files, and exit with the correct error code
  def cleanup(self):
    # Print the results table again if a bunch of output was spewed to the screen between
    # tests as they were running
    if self.options.verbose or (self.num_failed != 0 and not self.options.quiet):
      print '\n\nFinal Test Results:\n' + ('-' * (TERM_COLS-1))
      for (test, output, result, timing, start, end) in sorted(self.test_table, key=lambda x: x[2], reverse=True):
        if self.options.show_directory:
          print printResult(test['relative_path'] + '/' + specs['test_name'].split('/')[-1], result, timing, start, end, self.options)
        else:
          print printResult(test['test_name'], result, timing, start, end, self.options)

    time = clock() - self.start_time
    print '-' * (TERM_COLS-1)
    print 'Ran %d tests in %.1f seconds' % (self.num_passed+self.num_failed, time)

    if self.num_passed:
      summary = '<g>%d passed</g>'
    else:
      summary = '<b>%d passed</b>'
    summary += ', <b>%d skipped</b>'
    if self.num_pending:
      summary += ', <c>%d pending</c>'
    else:
      summary += ', <b>%d pending</b>'
    if self.num_failed:
      summary += ', <r>%d FAILED</r>'
    else:
      summary += ', <b>%d failed</b>'

    # Mask off TestHarness error codes to report parser errors
    if self.error_code & Parser.getErrorCodeMask():
      summary += ', <r>FATAL PARSER ERROR</r>'

    print colorText( summary % (self.num_passed, self.num_skipped, self.num_pending, self.num_failed),  "", html = True, \
                     colored=self.options.colored, code=self.options.code )
    if self.options.pbs:
      print '\nYour PBS batch file:', self.options.pbs
    if self.file:
      self.file.close()

    if self.num_failed == 0:
      self.writeState(self.executable)

  def initialize(self, argv, app_name):
    # Initialize the parallel runner with how many tests to run in parallel
    self.runner = RunParallel(self, self.options.jobs, self.options.load)

    ## Save executable-under-test name to self.executable
    self.executable = os.getcwd() + '/' + app_name + '-' + self.options.method

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
    parser = argparse.ArgumentParser(description='A tool used to test MOOSE based applications')
    parser.add_argument('test_name', nargs=argparse.REMAINDER)
    parser.add_argument('--opt', action='store_const', dest='method', const='opt', help='test the app_name-opt binary')
    parser.add_argument('--dbg', action='store_const', dest='method', const='dbg', help='test the app_name-dbg binary')
    parser.add_argument('--devel', action='store_const', dest='method', const='devel', help='test the app_name-devel binary')
    parser.add_argument('--oprof', action='store_const', dest='method', const='oprof', help='test the app_name-oprof binary')
    parser.add_argument('--pro', action='store_const', dest='method', const='pro', help='test the app_name-pro binary')
    parser.add_argument('-j', '--jobs', nargs='?', metavar='int', action='store', type=int, dest='jobs', const=1, help='run test binaries in parallel')
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
    parser.add_argument('--skip-config-checks', action='store_true', dest='skip_config_checks', help='Skip configuration checks (all tests will run regardless of restrictions)')
    parser.add_argument('--parallel', '-p', nargs='?', action='store', type=int, dest='parallel', const=1, help='Number of processors to use when running mpiexec')
    parser.add_argument('--n-threads', nargs=1, action='store', type=int, dest='nthreads', default=1, help='Number of threads to use when running mpiexec')
    parser.add_argument('-d', action='store_true', dest='debug_harness', help='Turn on Test Harness debugging')
    parser.add_argument('--recover', action='store_true', dest='enable_recover', help='Run a test in recover mode')
    parser.add_argument('--valgrind', action='store_const', dest='valgrind_mode', const='NORMAL', help='Run normal valgrind tests')
    parser.add_argument('--valgrind-heavy', action='store_const', dest='valgrind_mode', const='HEAVY', help='Run heavy valgrind tests')
    parser.add_argument('--valgrind-max-fails', nargs=1, type=int, dest='valgrind_max_fails', default=5, help='The number of valgrind tests allowed to fail before any additional valgrind tests will run')
    parser.add_argument('--max-fails', nargs=1, type=int, dest='max_fails', default=50, help='The number of tests allowed to fail before any additional tests will run')
    parser.add_argument('--pbs', nargs='?', metavar='batch_file', dest='pbs', const='generate', help='Enable launching tests via PBS. If no batch file is specified one will be created for you')
    parser.add_argument('--pbs-cleanup', nargs=1, metavar='batch_file', help='Clean up the directories/files created by PBS. You must supply the same batch_file used to launch PBS.')
    parser.add_argument('--pbs-project', nargs=1, default='moose', help='Identify PBS job submission to specified project')
    parser.add_argument('--re', action='store', type=str, dest='reg_exp', help='Run tests that match --re=regular_expression')

    # Options that pass straight through to the executable
    parser.add_argument('--parallel-mesh', action='store_true', dest='parallel_mesh', help='Deprecated, use --distributed-mesh instead')
    parser.add_argument('--distributed-mesh', action='store_true', dest='distributed_mesh', help='Pass "--distributed-mesh" to executable')
    parser.add_argument('--error', action='store_true', help='Run the tests with warnings as errors (Pass "--error" to executable)')
    parser.add_argument('--error-unused', action='store_true', help='Run the tests with errors on unused parameters (Pass "--error-unused" to executable)')

    # Option to use for passing unwrapped options to the executable
    parser.add_argument('--cli-args', nargs='?', type=str, dest='cli_args', help='Append the following list of arguments to the command line (Encapsulate the command in quotes)')

    parser.add_argument('--dry-run', action='store_true', dest='dry_run', help="Pass --dry-run to print commands to run, but don't actually run them")

    outputgroup = parser.add_argument_group('Output Options', 'These options control the output of the test harness. The sep-files options write output to files named test_name.TEST_RESULT.txt. All file output will overwrite old files')
    outputgroup.add_argument('-v', '--verbose', action='store_true', dest='verbose', help='show the output of every test')
    outputgroup.add_argument('-q', '--quiet', action='store_true', dest='quiet', help='only show the result of every test, don\'t show test output even if it fails')
    outputgroup.add_argument('--no-report', action='store_false', dest='report_skipped', help='do not report skipped tests')
    outputgroup.add_argument('--show-directory', action='store_true', dest='show_directory', help='Print test directory path in out messages')
    outputgroup.add_argument('-o', '--output-dir', nargs=1, metavar='directory', dest='output_dir', default='', help='Save all output files in the directory, and create it if necessary')
    outputgroup.add_argument('-f', '--file', nargs=1, action='store', dest='file', help='Write verbose output of each test to FILE and quiet output to terminal')
    outputgroup.add_argument('-x', '--sep-files', action='store_true', dest='sep_files', help='Write the output of each test to a separate file. Only quiet output to terminal. This is equivalant to \'--sep-files-fail --sep-files-ok\'')
    outputgroup.add_argument('--sep-files-ok', action='store_true', dest='ok_files', help='Write the output of each passed test to a separate file')
    outputgroup.add_argument('-a', '--sep-files-fail', action='store_true', dest='fail_files', help='Write the output of each FAILED test to a separate file. Only quiet output to terminal.')
    outputgroup.add_argument("--store-timing", action="store_true", dest="store_time", help="Store timing in the SQL database: $HOME/timingDB/timing.sqlite A parent directory (timingDB) must exist.")
    outputgroup.add_argument("--revision", nargs=1, action="store", type=str, dest="revision", help="The current revision being tested. Required when using --store-timing.")
    outputgroup.add_argument("--yaml", action="store_true", dest="yaml", help="Dump the parameters for the testers in Yaml Format")
    outputgroup.add_argument("--dump", action="store_true", dest="dump", help="Dump the parameters for the testers in GetPot Format")

    code = True
    if self.code.decode('hex') in argv:
      del argv[argv.index(self.code.decode('hex'))]
      code = False
    self.options = parser.parse_args(argv[1:])
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
    if opts.valgrind_mode and (opts.parallel > 1 or opts.nthreads > 1):
      print 'ERROR: --parallel and/or --threads can not be used with --valgrind'
      sys.exit(1)

    # Update any keys from the environment as necessary
    if not self.options.method:
      if os.environ.has_key('METHOD'):
        self.options.method = os.environ['METHOD']
      else:
        self.options.method = 'opt'

    if not self.options.valgrind_mode:
      self.options.valgrind_mode = ''

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

    # When running heavy tests, we'll make sure we use --no-report
    if opts.heavy_tests:
      self.options.report_skipped = False

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

  def getOptions(self):
    return self.options

#################################################################################################################################
# The TestTimer TestHarness
# This method finds and stores timing for individual tests.  It is activated with --store-timing
#################################################################################################################################

CREATE_TABLE = """create table timing
(
  app_name text,
  test_name text,
  revision text,
  date int,
  seconds real,
  scale int,
  load real
);"""

class TestTimer(TestHarness):
  def __init__(self, argv, app_name, moose_dir):
    TestHarness.__init__(self, argv, app_name, moose_dir)
    try:
      from sqlite3 import dbapi2 as sqlite
    except:
      print 'Error: --store-timing requires the sqlite3 python module.'
      sys.exit(1)
    self.app_name = app_name
    self.db_file = self.options.dbFile
    if not self.db_file:
      home = os.environ['HOME']
      self.db_file = os.path.join(home, 'timingDB/timing.sqlite')
      if not os.path.exists(self.db_file):
        print 'Warning: creating new database at default location: ' + str(self.db_file)
        self.createDB(self.db_file)
      else:
        print 'Warning: Assuming database location ' + self.db_file

  def createDB(self, fname):
    from sqlite3 import dbapi2 as sqlite
    print 'Creating empty database at ' + fname
    con = sqlite.connect(fname)
    cr = con.cursor()
    cr.execute(CREATE_TABLE)
    con.commit()

  def preRun(self):
    from sqlite3 import dbapi2 as sqlite
    # Delete previous data if app_name and repo revision are found
    con = sqlite.connect(self.db_file)
    cr = con.cursor()
    cr.execute('delete from timing where app_name = ? and revision = ?', (self.app_name, self.options.revision))
    con.commit()

  # After the run store the results in the database
  def postRun(self, test, timing):
    from sqlite3 import dbapi2 as sqlite
    con = sqlite.connect(self.db_file)
    cr = con.cursor()

    timestamp = int(time.time())
    load = os.getloadavg()[0]

    # accumulate the test results
    data = []
    sum_time = 0
    num = 0
    parse_failed = False
    # Were only interested in storing scaled data
    if timing != None and test['scale_refine'] != 0:
      sum_time += float(timing)
      num += 1
      data.append( (self.app_name, test['test_name'].split('/').pop(), self.options.revision, timestamp, timing, test['scale_refine'], load) )
    # Insert the data into the database
    cr.executemany('insert into timing values (?,?,?,?,?,?,?)', data)
    con.commit()
