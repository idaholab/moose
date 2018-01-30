#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
if sys.version_info[0:2] != (2, 7):
    print("python 2.7 is required to run the test harness")
    sys.exit(1)

import os, re, inspect, errno, copy
import shlex

from socket import gethostname
from FactorySystem.Factory import Factory
from FactorySystem.Parser import Parser
from FactorySystem.Warehouse import Warehouse
import util
import hit

import argparse
from timeit import default_timer as clock

def findTestRoot(start=os.getcwd(), method=os.environ.get('METHOD', 'opt')):
    rootdir = os.path.abspath(start)
    while os.path.dirname(rootdir) != rootdir:
        fname = os.path.join(rootdir, 'testroot')
        if os.path.exists(fname):
            with open(fname, 'r') as f:
                data = f.read()
            root = hit.parse(fname, data)
            args = []
            if root.find('cli_args'):
                args = shlex.split(root.param('cli_args'))
            # TODO: add check to see if the binary exists before returning. This can be used to
            # allow users to control fallthrough for e.g. individual module binaries vs. the
            # combined binary.
            return rootdir, root.param('app_name'), args
        rootdir = os.path.dirname(rootdir)
    raise RuntimeError('test root directory not found')

class TestHarness:

    @staticmethod
    def buildAndRun(argv, app_name, moose_dir):
        harness = TestHarness(argv, moose_dir, app_name=app_name)
        harness.findAndRunTests()
        sys.exit(harness.error_code)

    def __init__(self, argv, moose_dir, app_name=None):
        os.environ['MOOSE_DIR'] = moose_dir
        os.environ['PYTHONPATH'] = os.path.join(moose_dir, 'python') + ':' + os.environ.get('PYTHONPATH', '')

        if app_name:
            rootdir, app_name, args = '.', app_name, []
        else:
            rootdir, app_name, args = findTestRoot(start=os.getcwd())

        orig_cwd = os.getcwd()
        os.chdir(rootdir)
        argv = argv[:1] + args + argv[1:]

        self.factory = Factory()

        self.app_name = app_name

        # Build a Warehouse to hold the MooseObjects
        self.warehouse = Warehouse()

        # Get dependant applications and load dynamic tester plugins
        # If applications have new testers, we expect to find them in <app_dir>/scripts/TestHarness/testers
        dirs = [os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))]
        sys.path.append(os.path.join(moose_dir, 'framework', 'scripts'))   # For find_dep_apps.py

        # Use the find_dep_apps script to get the dependant applications for an app
        import find_dep_apps
        depend_app_dirs = find_dep_apps.findDepApps(app_name, use_current_only=True)
        dirs.extend([os.path.join(my_dir, 'scripts', 'TestHarness') for my_dir in depend_app_dirs.split('\n')])

        # Finally load the plugins!
        self.factory.loadPlugins(dirs, 'testers', "IS_TESTER")

        self._infiles = ['tests', 'speedtests']
        self.parse_errors = []
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

        # Failed Tests file object
        self.writeFailedTest = None

        # Parse arguments
        self.parseCLArgs(argv)

        checks = {}
        checks['platform'] = util.getPlatforms()
        checks['submodules'] = util.getInitializedSubmodules(self.run_tests_dir)
        checks['exe_objects'] = None # This gets calculated on demand

        # The TestHarness doesn't strictly require the existence of libMesh in order to run. Here we allow the user
        # to select whether they want to probe for libMesh configuration options.
        if self.options.skip_config_checks:
            checks['compiler'] = set(['ALL'])
            checks['petsc_version'] = 'N/A'
            checks['petsc_version_release'] = set(['ALL'])
            checks['slepc_version'] = 'N/A'
            checks['library_mode'] = set(['ALL'])
            checks['mesh_mode'] = set(['ALL'])
            checks['dtk'] = set(['ALL'])
            checks['unique_ids'] = set(['ALL'])
            checks['vtk'] = set(['ALL'])
            checks['tecplot'] = set(['ALL'])
            checks['dof_id_bytes'] = set(['ALL'])
            checks['petsc_debug'] = set(['ALL'])
            checks['curl'] = set(['ALL'])
            checks['tbb'] = set(['ALL'])
            checks['superlu'] = set(['ALL'])
            checks['slepc'] = set(['ALL'])
            checks['unique_id'] = set(['ALL'])
            checks['cxx11'] = set(['ALL'])
            checks['asio'] =  set(['ALL'])
            checks['boost'] = set(['ALL'])
        else:
            checks['compiler'] = util.getCompilers(self.libmesh_dir)
            checks['petsc_version'] = util.getPetscVersion(self.libmesh_dir)
            checks['petsc_version_release'] = util.getLibMeshConfigOption(self.libmesh_dir, 'petsc_version_release')
            checks['slepc_version'] = util.getSlepcVersion(self.libmesh_dir)
            checks['library_mode'] = util.getSharedOption(self.libmesh_dir)
            checks['mesh_mode'] = util.getLibMeshConfigOption(self.libmesh_dir, 'mesh_mode')
            checks['dtk'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'dtk')
            checks['unique_ids'] = util.getLibMeshConfigOption(self.libmesh_dir, 'unique_ids')
            checks['vtk'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'vtk')
            checks['tecplot'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'tecplot')
            checks['dof_id_bytes'] = util.getLibMeshConfigOption(self.libmesh_dir, 'dof_id_bytes')
            checks['petsc_debug'] = util.getLibMeshConfigOption(self.libmesh_dir, 'petsc_debug')
            checks['curl'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'curl')
            checks['tbb'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'tbb')
            checks['superlu'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'superlu')
            checks['slepc'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'slepc')
            checks['unique_id'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'unique_id')
            checks['cxx11'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'cxx11')
            checks['asio'] =  util.getIfAsioExists(self.moose_dir)
            checks['boost'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'boost')

        # Override the MESH_MODE option if using the '--distributed-mesh'
        # or (deprecated) '--parallel-mesh' option.
        if (self.options.parallel_mesh == True or self.options.distributed_mesh == True) or \
              (self.options.cli_args != None and \
               (self.options.cli_args.find('--parallel-mesh') != -1 or self.options.cli_args.find('--distributed-mesh') != -1)):

            option_set = set(['ALL', 'DISTRIBUTED'])
            checks['mesh_mode'] = option_set

        method = set(['ALL', self.options.method.upper()])
        checks['method'] = method

        # This is so we can easily pass checks around to any scheduler plugin
        self.options._checks = checks

        self.initialize(argv, app_name)

        os.chdir(orig_cwd)

    """
    Recursively walks the current tree looking for tests to run
    Error codes:
    0x0  - Success
    0x80 - TestHarness error
    """
    def findAndRunTests(self, find_only=False):
        self.error_code = 0x0
        self.preRun()
        self.start_time = clock()
        launched_tests = []
        if self.options.input_file_name != '':
            self._infiles = self.options.input_file_name.split(',')

        try:
            self.base_dir = os.getcwd()
            for dirpath, dirnames, filenames in os.walk(self.base_dir, followlinks=True):
                # Prune submdule paths when searching for tests

                dir_name = os.path.basename(dirpath)
                if (self.base_dir != dirpath and os.path.exists(os.path.join(dirpath, '.git'))) or dir_name in [".git", ".svn"]:
                    dirnames[:] = []
                    filenames[:] = []

                # walk into directories that aren't contrib directories
                if "contrib" not in os.path.relpath(dirpath, os.getcwd()):
                    for file in filenames:
                        # See if there were other arguments (test names) passed on the command line
                        if file in self._infiles \
                               and os.path.abspath(os.path.join(dirpath, file)) not in launched_tests:

                            if self.prunePath(file):
                                continue

                            saved_cwd = os.getcwd()
                            sys.path.append(os.path.abspath(dirpath))
                            os.chdir(dirpath)

                            # Get the testers for this test
                            testers = self.createTesters(dirpath, file, find_only)

                            # Some 'test' files generate empty tests. Such as an entirely commented out
                            # test block:
                            #
                            # [Tests]
                            #   # [./test]
                            #   # [../]
                            # []
                            #
                            # We do not want to send an empty list of testers to the scheduler.
                            if len(testers):
                                # Schedule the testers for immediate execution
                                self.scheduler.schedule(testers)

                            # record these launched test to prevent this test from launching again
                            # due to os.walk following symbolic links
                            launched_tests.append(os.path.join(dirpath, file))

                            os.chdir(saved_cwd)
                            sys.path.pop()

            # Wait for all the tests to complete
            self.scheduler.waitFinish()

            self.cleanup()

            # flags for the TestHarness start at the high bit
            if self.num_failed or self.scheduler.schedulerError():
                self.error_code = self.error_code | 0x80

        except KeyboardInterrupt:
            # Attempt to kill jobs currently running
            self.scheduler.killRemaining()

            if self.writeFailedTest != None:
                self.writeFailedTest.close()
            print('\nExiting due to keyboard interrupt...')
            sys.exit(1)

        return

   # Create and return list of tester objects. A tester is created by providing
    # abspath to basename (dirpath), and the test file in queustion (file)
    def createTesters(self, dirpath, file, find_only):
        # Build a Parser to parse the objects
        parser = Parser(self.factory, self.warehouse)

        # Parse it
        parser.parse(file)
        self.parse_errors.extend(parser.errors)

        # Retrieve the tests from the warehouse
        testers = self.warehouse.getActiveObjects()

        # Augment the Testers with additional information directly from the TestHarness
        for tester in testers:
            self.augmentParameters(file, tester)

        # Short circuit this loop if we've only been asked to parse Testers
        # Note: The warehouse will accumulate all testers in this mode
        if find_only:
            self.warehouse.markAllObjectsInactive()
            return []

        # Clear out the testers, we won't need them to stick around in the warehouse
        self.warehouse.clear()

        if self.options.enable_recover:
            testers = self.appendRecoverableTests(testers)

        return testers

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
        first_directory = relative_path.split(os.path.sep)[1] # Get first directory
        for infile in self._infiles:
            if infile in relative_path:
                relative_path = relative_path.replace('/' + infile + '/', ':')
                break
        relative_path = re.sub('^[/:]*', '', relative_path)  # Trim slashes and colons
        formatted_name = relative_path + '.' + tester.name()

        params['test_name'] = formatted_name
        params['test_dir'] = test_dir
        params['relative_path'] = relative_path
        params['executable'] = self.executable
        params['hostname'] = self.host_name
        params['moose_dir'] = self.moose_dir
        params['base_dir'] = self.base_dir
        params['first_directory'] = first_directory

        if params.isValid('prereq'):
            if type(params['prereq']) != list:
                print("Option 'prereq' needs to be of type list in " + params['test_name'])
                sys.exit(1)
            params['prereq'] = [relative_path.replace('/tests/', '') + '.' + item for item in params['prereq']]

        # Double the alloted time for tests when running with the valgrind option
        tester.setValgrindMode(self.options.valgrind_mode)

        # When running in valgrind mode, we end up with a ton of output for each failed
        # test.  Therefore, we limit the number of fails...
        if self.options.valgrind_mode and self.num_failed > self.options.valgrind_max_fails:
            tester.setStatus('Max Fails Exceeded', tester.bucket_fail)
        elif self.num_failed > self.options.max_fails:
            tester.setStatus('Max Fails Exceeded', tester.bucket_fail)
        elif tester.parameters().isValid('have_errors') and tester.parameters()['have_errors']:
            tester.setStatus('Parser Error', tester.bucket_fail)

    # This method splits a lists of tests into two pieces each, the first piece will run the test for
    # approx. half the number of timesteps and will write out a restart file.  The second test will
    # then complete the run using the MOOSE recover option.
    def appendRecoverableTests(self, testers):
        new_tests = []

        for part1 in testers:
            if part1.parameters()['recover'] == True and not part1.parameters()['check_input']:
                # Clone the test specs
                part2 = copy.deepcopy(part1)

                # Part 1:
                part1_params = part1.parameters()
                part1_params['test_name'] += '_part1'
                part1_params['cli_args'].append('--half-transient')
                if self.options.recoversuffix == 'cpr':
                    part1_params['cli_args'].append('Outputs/checkpoint=true')
                if self.options.recoversuffix == 'cpa':
                    part1_params['cli_args'].append('Outputs/out/type=Checkpoint')
                    part1_params['cli_args'].append('Outputs/out/binary=false')
                part1_params['skip_checks'] = True

                # Part 2:
                part2_params = part2.parameters()
                part2_params['prereq'].append(part1.parameters()['test_name'])
                part2_params['delete_output_before_running'] = False
                part2_params['cli_args'].append('--recover --recoversuffix ' + self.options.recoversuffix)
                part2.addCaveats('recover')

                new_tests.append(part2)

            elif part1.parameters()['recover'] == True and part1.parameters()['check_input']:
                part1.setStatus('SYNTAX ONLY TEST', part1.bucket_silent)

        testers.extend(new_tests)
        return testers

    def checkExpectError(self, output, expect_error):
        if re.search(expect_error, output, re.MULTILINE | re.DOTALL) == None:
            return False
        else:
            return True

    # Method contianing logic on whether or not we should print results of given tester
    def canPrint(self, tester):
        if tester.isSkipped() and self.options.report_skipped is False:
            return False

        elif tester.isSilent() or (tester.isDeleted() and not self.options.extra_info):
            return False

        return True

    # Format the caveats contained in tester so they are easy to read when printed
    def formatCaveats(self, tester):
        # PASS and DRY_RUN fall into this catagory
        if tester.didPass():
            result = tester.success_message
            if self.options.extra_info:
                for check in self.options._checks.keys():
                    if tester.specs.isValid(check) and not 'ALL' in tester.specs[check]:
                        tester.addCaveats(check)

        # FAIL, DIFF and DELETED fall into this catagory
        elif tester.didFail() or tester.didDiff() or tester.isDeleted():
            result = 'FAILED (%s)' % tester.getStatusMessage()

        # Silent tests have no results
        elif tester.isSilent():
            result = ''

        # Some other finished status... skipped, queued, etc
        else:
            result = tester.getStatusMessage()

        return result

    # Print and return formatted current tester status output
    def printResult(self, tester_data):
        """ Method to print a testers status to the screen """
        tester = tester_data.getTester()
        formatted_results = None

        # Print what ever status the tester has at the time
        if self.canPrint(tester):
            if self.options.verbose or (tester.didFail() and not self.options.quiet):
                output = 'Working Directory: ' + tester.getTestDir() + '\nRunning command: ' + tester.getCommand(self.options) + '\n'

                output += tester_data.getOutput()
                output = output.replace('\r', '\n')  # replace the carriage returns with newlines
                lines = output.split('\n')

                # Obtain color based on test status
                color = tester.getColor()

                if output != '':
                    test_name = util.colorText(tester.getTestName()  + ": ", color, colored=self.options.colored, code=self.options.code)
                    output = test_name + ("\n" + test_name).join(lines)
                    print(output)

            formatted_results = self.formatCaveats(tester)
            print(util.formatResult(tester_data, formatted_results, self.options))
        return formatted_results

    def handleTestStatus(self, tester_data):
        """ Method to handle a testers status """
        tester = tester_data.getTester()

        # print and store those results
        result = self.printResult(tester_data)

        # Test is finished and had some results to print
        if tester.isFinished() and self.canPrint(tester):
            timing = tester_data.getTiming()

            # Store these results to a table we will use when we print final results
            self.test_table.append( (tester_data, result, timing) )

            self.postRun(tester.specs, timing)
            # Tally the results of this test to be used in our Final Test Results footer
            if tester.isSkipped():
                self.num_skipped += 1
            elif tester.didPass():
                self.num_passed += 1
            elif tester.isQueued() or tester.isWaiting():
                self.num_pending += 1
            else:
                self.num_failed += 1

            # Write results to a file if asked to do so
            if not tester.isSkipped():
                if not tester.didPass() and not self.options.failed_tests:
                    self.writeFailedTest.write(tester.getTestName() + '\n')

                if self.options.file:
                    self.file.write(util.formatResult( tester_data, result, self.options, color=False) + '\n')

                if self.options.sep_files or (self.options.fail_files and not tester.didPass()) or (self.options.ok_files and tester.didPass()):
                    fname = os.path.join(tester.getTestDir(), tester.getTestName().split('/')[-1] + '.' + result[:6] + '.txt')
                    f = open(fname, 'w')
                    f.write(util.formatResult( tester_data, tester_data.getOutput(), self.options, color=False) + '\n')
                    f.close()

    # Print final results, close open files, and exit with the correct error code
    def cleanup(self):
        # Print the results table again if a bunch of output was spewed to the screen between
        # tests as they were running
        if len(self.parse_errors) > 0:
            print('\n\nParser Errors:\n' + ('-' * (util.TERM_COLS)))
            for err in self.parse_errors:
                print(util.colorText(err, 'RED', html=True, colored=self.options.colored, code=self.options.code))

        if (self.options.verbose or (self.num_failed != 0 and not self.options.quiet)) and not self.options.dry_run:
            print('\n\nFinal Test Results:\n' + ('-' * (util.TERM_COLS)))
            for (tester_data, result, timing) in sorted(self.test_table, key=lambda x: x[1], reverse=True):
                print(util.formatResult(tester_data, result, self.options))

        time = clock() - self.start_time

        print('-' * (util.TERM_COLS))

        # Mask off TestHarness error codes to report parser errors
        fatal_error = ''
        if self.error_code:
            fatal_error += ', <r>FATAL TEST HARNESS ERROR</r>'
        if len(self.parse_errors) > 0:
            fatal_error += ', <r>FATAL PARSER ERROR</r>'
            self.error_code = 1

        # Alert the user to their session file
        if self.options.queueing:
            print 'Your session file is %s' % self.options.session_file

        # Print a different footer when performing a dry run
        if self.options.dry_run:
            print('Processed %d tests in %.1f seconds' % (self.num_passed+self.num_skipped, time))
            summary = '<b>%d would run</b>'
            summary += ', <b>%d would be skipped</b>'
            summary += fatal_error
            print(util.colorText( summary % (self.num_passed, self.num_skipped),  "", html = True, \
                             colored=self.options.colored, code=self.options.code ))

        else:
            print('Ran %d tests in %.1f seconds' % (self.num_passed+self.num_failed, time))

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
            summary += fatal_error

            print(util.colorText( summary % (self.num_passed, self.num_skipped, self.num_pending, self.num_failed),  "", html = True, \
                             colored=self.options.colored, code=self.options.code ))

        if self.file:
            self.file.close()

        # Close the failed_tests file
        if self.writeFailedTest != None:
            self.writeFailedTest.close()

    def initialize(self, argv, app_name):
        # Load the scheduler plugins
        self.factory.loadPlugins([os.path.join(self.moose_dir, 'python', 'TestHarness')], 'schedulers', "IS_SCHEDULER")

        # Set a global queueing flag
        self.options.queueing = False

        if self.options.pbs:
            self.options.queueing = True
            scheduler_plugin = 'RunPBS'

            # Unify the queueing file
            self.options.session_file = self.options.pbs

        # User is wanting to clean up old queue sessions
        elif self.options.queue_cleanup:
            scheduler_plugin = 'QueueManager'
            self.options.session_file = self.options.queue_cleanup

        # The default scheduler plugin
        else:
            scheduler_plugin = 'RunParallel'

        # Augment the Scheduler params with plugin params
        plugin_params = self.factory.validParams(scheduler_plugin)

        # Set Scheduler specific params based on some provided options.arguments
        plugin_params['max_processes'] = self.options.jobs
        plugin_params['average_load'] = self.options.load

        # Create the scheduler
        self.scheduler = self.factory.create(scheduler_plugin, self, plugin_params)

        ## Save executable-under-test name to self.executable
        self.executable = os.getcwd() + '/' + app_name + '-' + self.options.method

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
        parser = argparse.ArgumentParser(description='A tool used to test MOOSE based applications')
        parser.add_argument('test_name', nargs=argparse.REMAINDER)
        parser.add_argument('--opt', action='store_const', dest='method', const='opt', help='test the app_name-opt binary')
        parser.add_argument('--dbg', action='store_const', dest='method', const='dbg', help='test the app_name-dbg binary')
        parser.add_argument('--devel', action='store_const', dest='method', const='devel', help='test the app_name-devel binary')
        parser.add_argument('--oprof', action='store_const', dest='method', const='oprof', help='test the app_name-oprof binary')
        parser.add_argument('--pro', action='store_const', dest='method', const='pro', help='test the app_name-pro binary')
        parser.add_argument('--run', type=str, default='', dest='run', help='only run tests of the specified of tag(s)')
        parser.add_argument('--ignore', nargs='?', action='store', metavar='caveat', dest='ignored_caveats', const='all', type=str, help='ignore specified caveats when checking if a test should run: (--ignore "method compiler") Using --ignore with out a conditional will ignore all caveats')
        parser.add_argument('-j', '--jobs', nargs='?', metavar='int', action='store', type=int, dest='jobs', const=1, help='run test binaries in parallel')
        parser.add_argument('-e', action='store_true', dest='extra_info', help='Display "extra" information including all caveats and deleted tests')
        parser.add_argument('-c', '--no-color', action='store_false', dest='colored', help='Do not show colored output')
        parser.add_argument('--color-first-directory', action='store_true', dest='color_first_directory', help='Color first directory')
        parser.add_argument('--heavy', action='store_true', dest='heavy_tests', help='Run tests marked with HEAVY : True')
        parser.add_argument('--all-tests', action='store_true', dest='all_tests', help='Run normal tests and tests marked with HEAVY : True')
        parser.add_argument('-g', '--group', action='store', type=str, dest='group', default='ALL', help='Run only tests in the named group')
        parser.add_argument('--not_group', action='store', type=str, dest='not_group', help='Run only tests NOT in the named group')
        parser.add_argument('--dbfile', nargs='?', action='store', dest='dbFile', help='Location to timings data base file. If not set, assumes $HOME/timingDB/timing.sqlite')
        parser.add_argument('-l', '--load-average', action='store', type=float, dest='load', help='Do not run additional tests if the load average is at least LOAD')
        parser.add_argument('-t', '--timing', action='store_true', dest='timing', help='Report Timing information for passing tests')
        parser.add_argument('-s', '--scale', action='store_true', dest='scaling', help='Scale problems that have SCALE_REFINE set')
        parser.add_argument('-i', nargs=1, action='store', type=str, dest='input_file_name', default='', help='The default test specification file to look for (default="tests").')
        parser.add_argument('--libmesh_dir', nargs=1, action='store', type=str, dest='libmesh_dir', help='Currently only needed for bitten code coverage')
        parser.add_argument('--skip-config-checks', action='store_true', dest='skip_config_checks', help='Skip configuration checks (all tests will run regardless of restrictions)')
        parser.add_argument('--parallel', '-p', nargs='?', action='store', type=int, dest='parallel', const=1, help='Number of processors to use when running mpiexec')
        parser.add_argument('--n-threads', nargs=1, action='store', type=int, dest='nthreads', default=1, help='Number of threads to use when running mpiexec')
        parser.add_argument('-d', action='store_true', dest='debug_harness', help='Turn on Test Harness debugging')
        parser.add_argument('--recover', action='store_true', dest='enable_recover', help='Run a test in recover mode')
        parser.add_argument('--recoversuffix', action='store', type=str, default='cpr', dest='recoversuffix', help='Set the file suffix for recover mode')
        parser.add_argument('--valgrind', action='store_const', dest='valgrind_mode', const='NORMAL', help='Run normal valgrind tests')
        parser.add_argument('--valgrind-heavy', action='store_const', dest='valgrind_mode', const='HEAVY', help='Run heavy valgrind tests')
        parser.add_argument('--valgrind-max-fails', nargs=1, type=int, dest='valgrind_max_fails', default=5, help='The number of valgrind tests allowed to fail before any additional valgrind tests will run')
        parser.add_argument('--max-fails', nargs=1, type=int, dest='max_fails', default=50, help='The number of tests allowed to fail before any additional tests will run')
        parser.add_argument('--re', action='store', type=str, dest='reg_exp', help='Run tests that match --re=regular_expression')
        parser.add_argument('--failed-tests', action='store_true', dest='failed_tests', help='Run tests that previously failed')
        parser.add_argument('--check-input', action='store_true', dest='check_input', help='Run check_input (syntax) tests only')
        parser.add_argument('--no-check-input', action='store_true', dest='no_check_input', help='Do not run check_input (syntax) tests')

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
        outputgroup.add_argument("--testharness-unittest", action="store_true", help="Run the TestHarness unittests that test the TestHarness.")
        outputgroup.add_argument("--yaml", action="store_true", dest="yaml", help="Dump the parameters for the testers in Yaml Format")
        outputgroup.add_argument("--dump", action="store_true", dest="dump", help="Dump the parameters for the testers in GetPot Format")
        outputgroup.add_argument("--no-trimmed-output", action="store_true", dest="no_trimmed_output", help="Do not trim the output")

        queuegroup = parser.add_argument_group('Queue Options', 'Options controlling which queue manager to use')
        queuegroup.add_argument('--pbs', nargs='?', action='store', type=str, metavar='session_name', const='generate', help='Use PBS as your scheduler. Optional: Supply a name to identify this session with. If session exists, the scheduler will instead display the results of that session.')
        queuegroup.add_argument('--queue-project', nargs=1, action='store', type=str, default='moose', metavar='project', help='Identify your PBS job with this project (default:  moose)')
        queuegroup.add_argument('--queue-cleanup', nargs=1, metavar='session_name', help='Clean up directories/files created by session')

        code = True
        if self.code.decode('hex') in argv:
            del argv[argv.index(self.code.decode('hex'))]
            code = False
        self.options = parser.parse_args(argv[1:])
        self.tests = self.options.test_name
        self.options.code = code

        self.options.runtags = [tag for tag in self.options.run.split(',') if tag != '']

        # Convert all list based options of length one to scalars
        for key, value in vars(self.options).items():
            if type(value) == list and len(value) == 1:
                setattr(self.options, key, value[0])

        # If attempting to test only failed_tests, open the .failed_tests file and create a list object
        # otherwise, open the failed_tests file object for writing (clobber).
        failed_tests_file = os.path.join(os.getcwd(), '.failed_tests')
        if self.options.failed_tests:
            with open(failed_tests_file, 'r') as tmp_failed_tests:
                self.options._test_list = tmp_failed_tests.read().split('\n')
        else:
            self.writeFailedTest = open(failed_tests_file, 'w')

        self.checkAndUpdateCLArgs()

    ## Called after options are parsed from the command line
    # Exit if options don't make any sense, print warnings if they are merely weird
    def checkAndUpdateCLArgs(self):
        opts = self.options
        if opts.output_dir and not (opts.file or opts.sep_files or opts.fail_files or opts.ok_files):
            print('WARNING: --output-dir is specified but no output files will be saved, use -f or a --sep-files option')
        if opts.group == opts.not_group:
            print('ERROR: The group and not_group options cannot specify the same group')
            sys.exit(1)
        if opts.valgrind_mode and (opts.parallel > 1 or opts.nthreads > 1):
            print('ERROR: --parallel and/or --threads can not be used with --valgrind')
            sys.exit(1)
        if opts.queue_cleanup and opts.pbs:
            print('ERROR: --queue-cleanup and --pbs can not be used together')
            sys.exit(1)
        if opts.check_input and opts.no_check_input:
            print('ERROR: --check-input and --no-check-input can not be used together')
            sys.exit(1)
        if opts.check_input and opts.enable_recover:
            print('ERROR: --check-input and --recover can not be used together')
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
        elif self.options.queue_cleanup:
            self.scheduler.cleanUp()
            sys.exit(0)

    def getOptions(self):
        return self.options
