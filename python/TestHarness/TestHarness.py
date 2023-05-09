#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import itertools
import platform
import os, re, inspect, errno, copy, json
import shlex
from . import RaceChecker
import subprocess
import shutil

from socket import gethostname
from FactorySystem.Factory import Factory
from FactorySystem.Parser import Parser
from FactorySystem.Warehouse import Warehouse
from . import util
import pyhit

import argparse
from timeit import default_timer as clock

def readTestRoot(fname):

    root = pyhit.load(fname)
    args = root.get('run_tests_args', '').split()

    # TODO: add check to see if the binary exists before returning. This can be used to
    # allow users to control fallthrough for e.g. individual module binaries vs. the
    # combined binary.
    return root['app_name'], args, root

def findTestRoot(start=os.getcwd(), method=os.environ.get('METHOD', 'opt')):
    rootdir = os.path.abspath(start)
    while os.path.dirname(rootdir) != rootdir:
        fname = os.path.join(rootdir, 'testroot')
        if os.path.exists(fname):
            app_name, args, hit_node = readTestRoot(fname)
            return rootdir, app_name, args, hit_node
        rootdir = os.path.dirname(rootdir)
    raise RuntimeError('test root directory not found in "{}"'.format(start))

# This function finds a file in the herd trunk containing all the possible applications
# that may be built with an "up" target.  If passed the value ROOT it will simply
# return the root directory
def findDepApps(dep_names, use_current_only=False):
    dep_name = dep_names.split('~')[0]

    app_dirs = []
    moose_apps = ['framework', 'moose', 'test', 'unit', 'modules', 'examples']
    apps = []

    # First see if we are in a git repo
    p = subprocess.Popen('git rev-parse --show-cdup', stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    p.wait()
    if p.returncode == 0:
        git_dir = p.communicate()[0].decode('utf-8')
        root_dir = os.path.abspath(os.path.join(os.getcwd(), git_dir)).rstrip()

        # Assume that any application we care about is always a peer
        dir_to_append = '.' if use_current_only else '..'
        app_dirs.append(os.path.abspath(os.path.join(root_dir, dir_to_append)))

    # Now see if we can find .build_apps in a parent directory from where we are at, usually "projects"
    restrict_file = '.build_apps'
    restrict_file_path = ''
    restrict_dir = ''

    next_dir = os.getcwd()
    for i in range(4):
        next_dir = os.path.join(next_dir, "..")
        if os.path.isfile(os.path.join(next_dir, restrict_file)):
            restrict_file_path = os.path.join(next_dir, restrict_file)
            break
    if restrict_file_path != '':
        restrict_dir = os.path.dirname(os.path.abspath(restrict_file_path))
        app_dirs.append(restrict_dir)

    # Make sure that we found at least one directory to search
    if len(app_dirs) == 0:
        return ''

    # unique paths to search
    unique_dirs = set()
    for dir in app_dirs:
        unique_dirs.add(os.path.abspath(dir))

    remove_dirs = set()
    # now strip common paths
    for dir1 in unique_dirs:
        for dir2 in unique_dirs:
            if dir1 == dir2:
                continue

            if dir1 in dir2:
                remove_dirs.add(dir2)
            elif dir2 in dir1:
                remove_dirs.add(dir1)
    # set difference
    unique_dirs = unique_dirs - remove_dirs

    if restrict_file_path != '':
        f = open(restrict_file_path)
        apps.extend(f.read().splitlines())
        f.close()

    # See which apps in this file are children or dependents of this app
    dep_apps = set()
    dep_dirs = set()

    # moose, elk and modules have special rules
    if dep_name == "moose":
        dep_app_re=re.compile(r"\bmoose\.mk\b")
    elif dep_name == "modules":
        dep_app_re=re.compile(r"\bmodules\.mk\b")
    elif dep_name == "elk":
        dep_app_re=re.compile(r"\belk(?:_module)?\.mk\b")
    else:
        dep_app_re=re.compile(r"^\s*APPLICATION_NAME\s*:=\s*"+dep_name,re.MULTILINE)

    ignores = ['.git', '.svn', '.libs', 'gold', 'src', 'include', 'contrib', 'tests', 'bak', 'tutorials']

    for dir in unique_dirs:
        startinglevel = dir.count(os.sep)
        for dirpath, dirnames, filenames in os.walk(dir, topdown=True):
            # Don't traverse too deep!
            if dirpath.count(os.sep) - startinglevel >= 2: # 2 levels outta be enough for anybody
                dirnames[:] = []

            # Don't traverse into ignored directories
            for ignore in ignores:
                if ignore in dirnames:
                    dirnames.remove(ignore)

            # Honor user ignored directories
            if os.path.isfile(os.path.join(dirpath, '.moose_ignore')):
                dirnames[:] = []
                continue

            # Don't traverse into submodules
            if os.path.isfile(os.path.join(dirpath, '.gitmodules')):
                f = open(os.path.join(dirpath, '.gitmodules'))
                content = f.read()
                f.close()
                sub_mods = re.findall(r'path = (\w+)', content)
                dirnames[:] = [x for x in dirnames if x not in sub_mods]

            potential_makefile = os.path.join(dirpath, 'Makefile')

            if os.path.isfile(potential_makefile):
                f = open(potential_makefile)
                lines = f.read()
                f.close()

                # We only want to build certain applications, look at the path to make a decision
                # If we are in trunk, we will honor .build_apps.  If we aren't, then we'll add it
                eligible_app = dirpath.split('/')[-1]

                if dep_app_re.search(lines) and ((len(apps) == 0 or eligible_app in apps) or ('/moose/' in dirpath and eligible_app in moose_apps)):
                    dep_apps.add(eligible_app)
                    dep_dirs.add(dirpath)

                    # Don't traverse once we've found a dependency
                    dirnames[:] = []

    # Now we need to filter out duplicate moose apps
    moose_dir = os.environ.get('MOOSE_DIR')
    return '\n'.join(dep_dirs)

class TestHarness:

    @staticmethod
    def buildAndRun(argv, app_name, moose_dir, moose_python=None):
        harness = TestHarness(argv, moose_dir, app_name=app_name, moose_python=moose_python)
        harness.findAndRunTests()
        sys.exit(harness.error_code)

    def __init__(self, argv, moose_dir, app_name=None, moose_python=None):
        if moose_python is None:
            self.moose_python_dir = os.path.join(moose_dir, "python")
        else:
            self.moose_python_dir = moose_python
        os.environ['MOOSE_DIR'] = moose_dir
        os.environ['PYTHONPATH'] = self.moose_python_dir + ':' + os.environ.get('PYTHONPATH', '')

        if app_name:
            rootdir, app_name, args, root_params = '.', app_name, [], pyhit.Node()
        else:
            rootdir, app_name, args, root_params = findTestRoot(start=os.getcwd())

        self._rootdir = rootdir
        self._orig_cwd = os.getcwd()
        os.chdir(rootdir)
        argv = argv[:1] + args + argv[1:]

        self.factory = Factory()

        self.app_name = app_name

        self.root_params = root_params

        # Build a Warehouse to hold the MooseObjects
        self.warehouse = Warehouse()

        # Get dependant applications and load dynamic tester plugins
        # If applications have new testers, we expect to find them in <app_dir>/scripts/TestHarness/testers
        dirs = [os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))]
        dirs.append(os.path.join(moose_dir, 'share', 'moose', 'python', 'TestHarness', 'testers'))

        # Use the find_dep_apps script to get the dependant applications for an app
        depend_app_dirs = findDepApps(app_name, use_current_only=True)
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
        self.code = b'2d2d6769726c2d6d6f6465'
        self.error_code = 0x0
        self.keyboard_talk = True
        self.results_file = '.previous_test_results.json'
        # Assume libmesh is a peer directory to MOOSE if not defined
        if "LIBMESH_DIR" in os.environ:
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
        checks['installed'] = util.checkInstalled(self.run_tests_dir)
        checks['exe_objects'] = None # This gets calculated on demand
        checks['registered_apps'] = None # This gets extracted on demand

        # The TestHarness doesn't strictly require the existence of libMesh in order to run. Here we allow the user
        # to select whether they want to probe for libMesh configuration options.
        if self.options.skip_config_checks:
            checks['compiler'] = set(['ALL'])
            checks['petsc_version'] = 'N/A'
            checks['petsc_version_release'] = 'N/A'
            checks['slepc_version'] = 'N/A'
            checks['exodus_version'] = 'N/A'
            checks['vtk_version'] = 'N/A'
            checks['library_mode'] = set(['ALL'])
            checks['mesh_mode'] = set(['ALL'])
            checks['dtk'] = set(['ALL'])
            checks['unique_ids'] = set(['ALL'])
            checks['vtk'] = set(['ALL'])
            checks['tecplot'] = set(['ALL'])
            checks['dof_id_bytes'] = set(['ALL'])
            checks['petsc_debug'] = set(['ALL'])
            checks['curl'] = set(['ALL'])
            checks['threading'] = set(['ALL'])
            checks['superlu'] = set(['ALL'])
            checks['mumps'] = set(['ALL'])
            checks['strumpack'] = set(['ALL'])
            checks['parmetis'] = set(['ALL'])
            checks['chaco'] = set(['ALL'])
            checks['party'] = set(['ALL'])
            checks['ptscotch'] = set(['ALL'])
            checks['slepc'] = set(['ALL'])
            checks['unique_id'] = set(['ALL'])
            checks['cxx11'] = set(['ALL'])
            checks['asio'] =  set(['ALL'])
            checks['boost'] = set(['ALL'])
            checks['fparser_jit'] = set(['ALL'])
            checks['libpng'] = set(['ALL'])
            checks['liborch'] = set(['ALL'])
            checks['libtorch_version'] = 'N/A'
        else:
            checks['compiler'] = util.getCompilers(self.libmesh_dir)
            checks['petsc_version'] = util.getPetscVersion(self.libmesh_dir)
            checks['petsc_version_release'] = util.getLibMeshConfigOption(self.libmesh_dir, 'petsc_version_release')
            checks['slepc_version'] = util.getSlepcVersion(self.libmesh_dir)
            checks['exodus_version'] = util.getExodusVersion(self.libmesh_dir)
            checks['vtk_version'] = util.getVTKVersion(self.libmesh_dir)
            checks['library_mode'] = util.getSharedOption(self.libmesh_dir)
            checks['mesh_mode'] = util.getLibMeshConfigOption(self.libmesh_dir, 'mesh_mode')
            checks['dtk'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'dtk')
            checks['unique_ids'] = util.getLibMeshConfigOption(self.libmesh_dir, 'unique_ids')
            checks['vtk'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'vtk')
            checks['tecplot'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'tecplot')
            checks['dof_id_bytes'] = util.getLibMeshConfigOption(self.libmesh_dir, 'dof_id_bytes')
            checks['petsc_debug'] = util.getLibMeshConfigOption(self.libmesh_dir, 'petsc_debug')
            checks['curl'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'curl')
            checks['threading'] =  util.getLibMeshThreadingModel(self.libmesh_dir)
            checks['superlu'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'superlu')
            checks['mumps'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'mumps')
            checks['strumpack'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'strumpack')
            checks['parmetis'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'parmetis')
            checks['chaco'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'chaco')
            checks['party'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'party')
            checks['ptscotch'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'ptscotch')
            checks['slepc'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'slepc')
            checks['unique_id'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'unique_id')
            checks['cxx11'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'cxx11')
            checks['asio'] =  util.getIfAsioExists(self.moose_dir)
            checks['boost'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'boost')
            checks['fparser_jit'] =  util.getLibMeshConfigOption(self.libmesh_dir, 'fparser_jit')
            checks['libpng'] = util.getMooseConfigOption(self.moose_dir, 'libpng')
            checks['libtorch'] = util.getMooseConfigOption(self.moose_dir, 'libtorch')
            checks['libtorch_version'] = util.getLibtorchVersion(self.moose_dir)

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

        os.chdir(self._orig_cwd)

    """
    Recursively walks the current tree looking for tests to run
    Error codes:
    0x0       - Success
    0x01-0x7F - Recoverable errors
    0x80-0xFF - Unrecoverable TestHarness errors
    """
    def findAndRunTests(self, find_only=False):
        self.error_code = 0x0
        self.preRun()
        self.start_time = clock()
        launched_tests = []
        if self.options.input_file_name != '':
            self._infiles = self.options.input_file_name.split(',')

        if self.options.spec_file and os.path.isdir(self.options.spec_file):
            search_dir = self.options.spec_file
        elif self.options.spec_file and os.path.isfile(self.options.spec_file):
            search_dir = os.path.dirname(self.options.spec_file)
            self._infiles = [os.path.basename(self.options.spec_file)]
        else:
            search_dir = os.getcwd()

        try:
            testroot_params = {}
            for dirpath, dirnames, filenames in os.walk(search_dir, followlinks=True):
                # Prune submdule paths when searching for tests

                dir_name = os.path.basename(dirpath)
                if (search_dir != dirpath and os.path.exists(os.path.join(dirpath, '.git'))) or dir_name in [".git", ".svn"]:
                    dirnames[:] = []
                    filenames[:] = []

                if self.options.use_subdir_exe and testroot_params and not dirpath.startswith(testroot_params["testroot_dir"]):
                    # Reset the params when we go outside the current testroot base directory
                    testroot_params = {}

                # walk into directories that aren't contrib directories
                if "contrib" not in os.path.relpath(dirpath, os.getcwd()):
                    for file in filenames:
                        if self.options.use_subdir_exe and file == "testroot":
                            # Rely on the fact that os.walk does a depth first traversal.
                            # Any directories below this one will use the executable specified
                            # in this testroot file unless it is overridden.
                            app_name, args, root_params = readTestRoot(os.path.join(dirpath, file))
                            full_app_name = app_name + "-" + self.options.method
                            if platform.system() == 'Windows':
                                full_app_name += '.exe'

                            testroot_params["executable"] = full_app_name
                            if shutil.which(full_app_name) is None:
                                testroot_params["executable"] = os.path.join(dirpath, full_app_name)

                            testroot_params["testroot_dir"] = dirpath
                            caveats = [full_app_name]
                            if args:
                                caveats.append("Ignoring args %s" % args)
                            testroot_params["caveats"] = caveats
                            testroot_params["root_params"] = root_params

                        # See if there were other arguments (test names) passed on the command line
                        if file in self._infiles \
                               and os.path.abspath(os.path.join(dirpath, file)) not in launched_tests:

                            if self.notMySpecFile(dirpath, file):
                                continue

                            saved_cwd = os.getcwd()
                            sys.path.append(os.path.abspath(dirpath))
                            os.chdir(dirpath)

                            # Create the testers for this test
                            testers = self.createTesters(dirpath, file, find_only, testroot_params)


                            # Schedule the testers (non blocking)
                            self.scheduler.schedule(testers)

                            # record these launched test to prevent this test from launching again
                            # due to os.walk following symbolic links
                            launched_tests.append(os.path.join(dirpath, file))

                            os.chdir(saved_cwd)
                            sys.path.pop()

            # Wait for all the tests to complete (blocking)
            self.scheduler.waitFinish()

            # TODO: this DOES NOT WORK WITH MAX FAILS (max fails is considered a scheduler error at the moment)
            if not self.scheduler.schedulerError():
                self.cleanup()

        except KeyboardInterrupt:
            # Attempt to kill jobs currently running
            self.scheduler.killRemaining(keyboard=True)
            self.keyboard_interrupt()
            sys.exit(1)

        return

    def keyboard_interrupt(self):
        """ Control how keyboard interrupt displays """
        if self.keyboard_talk:
            # Prevent multiple keyboard interrupt messages
            self.keyboard_talk = False
            print('\nExiting due to keyboard interrupt...')

    # Create and return list of tester objects. A tester is created by providing
    # abspath to basename (dirpath), and the test file in queustion (file)
    def createTesters(self, dirpath, file, find_only, testroot_params={}):
        # Build a Parser to parse the objects
        parser = Parser(self.factory, self.warehouse)

        # Parse it
        parser.parse(file, testroot_params.get("root_params", self.root_params))
        self.parse_errors.extend(parser.errors)

        # Retrieve the tests from the warehouse
        testers = self.warehouse.getActiveObjects()

        # Augment the Testers with additional information directly from the TestHarness
        for tester in testers:

            self.augmentParameters(file, tester, testroot_params)
            if testroot_params.get("caveats"):
                # Show what executable we are using if using a different testroot file
                tester.addCaveats(testroot_params["caveats"])

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

    def notMySpecFile(self, dirpath, filename):
        """ true if dirpath/filename does not match supplied --spec-file """
        if (self.options.spec_file
            and os.path.isfile(self.options.spec_file)
            and os.path.join(dirpath, filename) != self.options.spec_file):
            return True

    def augmentParameters(self, filename, tester, testroot_params={}):
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
        relative_hitpath = os.path.join(*params['hit_path'].split(os.sep)[2:])  # Trim root node "[Tests]"
        formatted_name = relative_path + '.' + relative_hitpath

        params['spec_file'] = filename
        params['test_name'] = formatted_name
        params['test_dir'] = test_dir
        params['relative_path'] = relative_path
        params['executable'] = testroot_params.get("executable", self.executable)
        params['hostname'] = self.host_name
        params['moose_dir'] = self.moose_dir
        params['moose_python_dir'] = self.moose_python_dir
        params['base_dir'] = self.base_dir
        params['first_directory'] = first_directory
        params['root_params'] = testroot_params.get("root_params", self.root_params)

        if params.isValid('prereq'):
            if type(params['prereq']) != list:
                print(("Option 'prereq' needs to be of type list in " + params['test_name']))
                sys.exit(1)
            elif (params['prereq'] != ['ALL']):
                params['prereq'] = [relative_path.replace('/tests/', '') + '.' + item for item in params['prereq']]

        # Double the alloted time for tests when running with the valgrind option
        tester.setValgrindMode(self.options.valgrind_mode)

        # When running in valgrind mode, we end up with a ton of output for each failed
        # test.  Therefore, we limit the number of fails...
        if self.options.valgrind_mode and self.num_failed > self.options.valgrind_max_fails:
            tester.setStatus(tester.fail, 'Max Fails Exceeded')
        elif self.num_failed > self.options.max_fails:
            tester.setStatus(tester.fail, 'Max Fails Exceeded')
        elif tester.parameters().isValid('have_errors') and tester.parameters()['have_errors']:
            tester.setStatus(tester.fail, 'Parser Error')

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
                part1.setStatus(part1.silent)

        testers.extend(new_tests)
        return testers

    def checkExpectError(self, output, expect_error):
        if re.search(expect_error, output, re.MULTILINE | re.DOTALL) == None:
            return False
        else:
            return True

    def printOutput(self, job, color):
        """ Method to print a testers output to the screen """
        output = ''
        # Print what ever status the tester has at the time
        if self.options.verbose or (job.isFail() and not self.options.quiet):
            output = 'Working Directory: ' + job.getTestDir() + '\nRunning command: ' + job.getCommand() + '\n'
            output += util.trimOutput(job, self.options)
            output = output.replace('\r', '\n')  # replace the carriage returns with newlines
            lines = output.split('\n')

            if output != '':
                test_name = util.colorText(job.getTestName()  + ": ", color, colored=self.options.colored, code=self.options.code)
                output = test_name + ("\n" + test_name).join(lines)
                print(output)
        return output

    def handleJobStatus(self, job):
        """
        The Scheduler is calling back the TestHarness to inform us of a status change.
        The job may or may not be finished yet (RUNNING), or failing, passing, etc.
        """
        if self.options.show_last_run and job.isSkip():
            return
        elif not job.isSilent():
            # Print results and perform any desired post job processing
            if job.isFinished():
                status, message, color, status_code, sort_value = job.getJointStatus()
                self.error_code = self.error_code | status_code

                # perform printing of application output if so desired
                self.printOutput(job, color)

                # Print status with caveats
                print((util.formatResult(job, self.options, caveats=True)))

                timing = job.getTiming()

                # Save these results for 'Final Test Result' summary
                self.test_table.append( (job, sort_value, timing) )
                self.postRun(job.specs, timing)

                if job.isSkip():
                    self.num_skipped += 1
                elif job.isPass():
                    self.num_passed += 1
                elif job.isFail():
                    self.num_failed += 1
                else:
                    self.num_pending += 1

            # Just print current status without saving results
            else:
                print((util.formatResult(job, self.options, result='RUNNING', caveats=False)))

    # Print final results, close open files, and exit with the correct error code
    def cleanup(self):
        if self.options.queue_cleanup and self.options.results_file:
            try:
                os.remove(self.options.results_file)
            except OSError:
                pass
            return

        # Print the results table again if a bunch of output was spewed to the screen between
        # tests as they were running
        if len(self.parse_errors) > 0:
            print(('\n\nParser Errors:\n' + ('-' * (util.TERM_COLS))))
            for err in self.parse_errors:
                print((util.colorText(err, 'RED', html=True, colored=self.options.colored, code=self.options.code)))

        if (self.options.verbose or (self.num_failed != 0 and not self.options.quiet)) and not self.options.dry_run:
            print(('\n\nFinal Test Results:\n' + ('-' * (util.TERM_COLS))))
            for (job, sort_value, timing) in sorted(self.test_table, key=lambda x: x[1]):
                print((util.formatResult(job, self.options, caveats=True)))

        time = clock() - self.start_time

        print(('-' * (util.TERM_COLS)))

        # Mask off TestHarness error codes to report parser errors
        fatal_error = ''
        if len(self.parse_errors) > 0:
            fatal_error += ', <r>FATAL PARSER ERROR</r>'
            self.error_code = self.error_code | 0x80

        # Alert the user to their session file
        if self.options.queueing and not self.options.dry_run:
            print(('Your session file is %s' % self.options.results_file))

        # Print a different footer when performing a dry run
        if self.options.dry_run:
            print(('Processed %d tests in %.1f seconds.' % (self.num_passed+self.num_skipped, time)))
            summary = '<b>%d would run</b>'
            summary += ', <b>%d would be skipped</b>'
            summary += fatal_error
            print((util.colorText( summary % (self.num_passed, self.num_skipped),  "", html = True, \
                             colored=self.options.colored, code=self.options.code )))

        else:
            num_nonzero_timing = sum(1 if float(tup[0].getTiming()) > 0 else 0 for tup in self.test_table)
            if num_nonzero_timing > 0:
                timing_max = max(float(tup[0].getTiming()) for tup in self.test_table)
                timing_avg = sum(float(tup[0].getTiming()) for tup in self.test_table) / num_nonzero_timing
            else:
                timing_max = 0
                timing_avg = 0
            print(('Ran %d tests in %.1f seconds. Average test time %.1f seconds, maximum test time %.1f seconds.' % (self.num_passed+self.num_failed, time, timing_avg, timing_max)))

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
            if self.scheduler.maxFailures():
                self.error_code = self.error_code | 0x80
                summary += '\n<r>MAX FAILURES REACHED</r>'

            summary += fatal_error

            print((util.colorText( summary % (self.num_passed, self.num_skipped, self.num_pending, self.num_failed),  "", html = True, \
                             colored=self.options.colored, code=self.options.code )))

            if self.options.longest_jobs:
                # Sort all jobs by run time
                sorted_tups = sorted(self.test_table, key=lambda tup: float(tup[0].getTiming()), reverse=True)

                print('\n%d longest running jobs:' % self.options.longest_jobs)
                print(('-' * (util.TERM_COLS)))

                # Copy the current options and force timing to be true so that
                # we get times when we call formatResult() below
                options_with_timing = copy.deepcopy(self.options)
                options_with_timing.timing = True

                for tup in sorted_tups[0:self.options.longest_jobs]:
                    job = tup[0]
                    if not job.isSkip() and float(job.getTiming()) > 0:
                        print(util.formatResult(job, options_with_timing, caveats=True))
                if len(sorted_tups) == 0 or float(sorted_tups[0][0].getTiming()) == 0:
                    print('No jobs were completed.')

                # The TestHarness receives individual jobs out of order (can't realistically use self.test_table)
                tester_dirs = {}
                dag_table = []
                for jobs, dag, thread_lock in self.scheduler.retrieveDAGs():
                    original_dag = dag.getOriginalDAG()
                    total_time = float(0.0)
                    tester = None
                    for tester in dag.topological_sort(original_dag):
                        if not tester.isSkip():
                            total_time += tester.getTiming()
                    if tester is not None:
                        tester_dirs[tester.getTestDir()] = (tester_dirs.get(tester.getTestDir(), 0) + total_time)
                for k, v in tester_dirs.items():
                    rel_spec_path = f'{os.path.sep}'.join(k.split(os.path.sep)[-2:])
                    dag_table.append([f'{rel_spec_path}{os.path.sep}{self._infiles[0]}', f'{v:.3f}'])

                sorted_table = sorted(dag_table, key=lambda dag_table: float(dag_table[1]), reverse=True)
                if sorted_table[0:self.options.longest_jobs]:
                    print(f'\n{self.options.longest_jobs} longest running folders:')
                    print(('-' * (util.TERM_COLS)))
                    # We can't use util.formatResults, as we are representing a group of testers
                    for group in sorted_table[0:self.options.longest_jobs]:
                        print(str(group[0]).ljust((util.TERM_COLS - (len(group[1]) + 4)), ' '), f'[{group[1]}s]')
                    print('\n')

            # Perform any write-to-disc operations
            self.writeResults()

    def writeResults(self):
        """ Don't update the results file when using the --failed-tests argument """
        if self.options.failed_tests or self.options.show_last_run:
            return

        """ write test results to disc in some fashion the user has requested """
        all_jobs = self.scheduler.retrieveJobs()

        # Gather and print the jobs with race conditions after the jobs are finished
        # and only run when running --diag.
        if self.options.pedantic_checks:
            checker = RaceChecker.RaceChecker(all_jobs)
            if checker.findRacePartners():
                # Print the unique racer conditions and adjust our error code.
                self.error_code = checker.printUniqueRacerSets()
            else:
                print("There are no race conditions.")

        # Record the input file name that was used
        self.options.results_storage['INPUT_FILE_NAME'] = self.options.input_file_name

        # Record that we are using --sep-files* options
        self.options.results_storage['SEP_FILES'] = (True if self.options.pbs else False
                                                     or self.options.ok_files
                                                     or self.options.fail_files
                                                     or self.options.sep_files)

        # Record the Scheduler Plugin used
        self.options.results_storage['SCHEDULER'] = self.scheduler.__class__.__name__

        # Write some useful data to our results_storage
        for job_group in all_jobs:
            for job in job_group:
                # If queueing, do not store silent results in session file
                if job.isSilent() and self.options.queueing:
                    continue

                status, message, message_color, status_code, sort_value = job.getJointStatus()

                # Create empty key based on TestDir, or re-inialize with existing data so we can append to it
                self.options.results_storage[job.getTestDir()] = self.options.results_storage.get(job.getTestDir(), {})

                # If output has been stored in separate files, don't make additional copies by
                # storing that data in this json results file (--pbs || --sep-files, etc options).
                output = '' if job.getOutputFile() else job.getOutput()

                self.options.results_storage[job.getTestDir()][job.getTestName()] = {'NAME'           : job.getTestNameShort(),
                                                                                     'LONG_NAME'      : job.getTestName(),
                                                                                     'TIMING'         : job.getTiming(),
                                                                                     'STATUS'         : status,
                                                                                     'STATUS_MESSAGE' : message,
                                                                                     'FAIL'           : job.isFail(),
                                                                                     'COLOR'          : message_color,
                                                                                     'CAVEATS'        : list(job.getCaveats()),
                                                                                     'OUTPUT'         : output,
                                                                                     'COMMAND'        : job.getCommand(),
                                                                                     'META_DATA'      : job.getMetaData()}

                # Additional data to store (overwrites any previous matching keys)
                self.options.results_storage[job.getTestDir()].update(job.getMetaData())

        if self.options.output_dir:
            self.options.results_file = os.path.join(self.options.output_dir, self.options.results_file)

        if self.options.results_storage and self.options.results_file:
            try:
                with open(self.options.results_file, 'w') as data_file:
                    json.dump(self.options.results_storage, data_file, indent=2)
            except UnicodeDecodeError:
                print('\nERROR: Unable to write results due to unicode decode/encode error')

                # write to a plain file to aid in reproducing error
                with open(self.options.results_file + '.unicode_error' , 'w') as f:
                    f.write(self.options.results_storage)

                sys.exit(1)
            except IOError:
                print('\nERROR: Unable to write results due to permissions')
                sys.exit(1)

        try:
            # Write one file, with verbose information (--file)
            if self.options.file:
                with open(os.path.join(self.output_dir, self.options.file), 'w') as f:
                    for job_group in all_jobs:
                        for job in job_group:
                            # Do not write information about silent tests
                            if job.isSilent():
                                continue

                            formated_results = util.formatResult( job, self.options, result=job.getOutput(), color=False)
                            f.write(formated_results + '\n')

            # Write a separate file for each test with verbose information (--sep-files, --sep-files-ok, --sep-files-fail)
            if ((self.options.ok_files and self.num_passed)
                or (self.options.fail_files and self.num_failed)):
                for job_group in all_jobs:
                    for job in job_group:
                        status, message, message_color, status_code, sort_value = job.getJointStatus()

                        if self.options.output_dir:
                            output_dir = self.options.output_dir
                        else:
                            output_dir = job.getTestDir()

                        output = ''
                        # Append input file contents to output
                        if self.options.include_input:
                            # This is a file i/o operation. We only want to do this once, and only if necessary
                            input_file = job.getInputFileContents()
                            if input_file:
                                output += "\n\nINPUT FILE:\n" + str(input_file)

                        output += "\n\nTEST OUTPUT:" + job.getOutput()
                        output_file = job.getOutputFile()
                        formated_results = util.formatResult(job, self.options, result=output, color=False)
                        if output_file:
                            with open(output_file, 'w') as f:
                                f.write(formated_results)

        except IOError:
            print('Permission error while writing results to disc')
            sys.exit(1)
        except:
            print('Error while writing results to disc')
            sys.exit(1)

    def initialize(self, argv, app_name):
        # Load the scheduler plugins
        plugin_paths = [os.path.join(self.moose_dir, 'python', 'TestHarness'), os.path.join(self.moose_dir, 'share', 'moose', 'python', 'TestHarness')]
        self.factory.loadPlugins(plugin_paths, 'schedulers', "IS_SCHEDULER")

        self.options.queueing = False
        if self.options.pbs:
            # original_storage will become the results file for each test being launched by PBS, and will be
            # saved in the same directory as the test spec file. This is so we can launch multiple 'run_tests'
            # without clobbering the parent results_file. Meanwhile, the new results_file is going to be
            # renamed to whatever the user decided to identify their PBS launch with.
            self.original_storage = self.options.results_file
            self.options.results_file = os.path.abspath(self.options.pbs)
            self.options.queueing = True
            scheduler_plugin = 'RunPBS'

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

        # Save executable-under-test name to self.executable
        exec_suffix = 'Windows' if platform.system() == 'Windows' else ''
        self.executable = app_name + '-' + self.options.method + exec_suffix

        # if the executable has a slash - assume it is a file path
        if '/' in app_name:
            self.executable = os.path.abspath(self.executable)
        # look for executable in PATH - if not there, check other places.
        elif os.path.exists(os.path.join(os.getcwd(), self.executable)):
            # it's in the current working directory
            self.executable = os.getcwd() + '/' + self.executable
        elif os.path.exists(os.path.join(self._rootdir, self.executable)):
            # it's in the testroot file's directory
            self.executable = self.executable
            # we may be hopping around between multiple (module)
            # subdirectories of tests - so the executable needs to be an
            # absolute path.
            self.executable = os.path.abspath(os.path.join(self._rootdir, self.executable))
        else:
            # it's (hopefully) in an installed location
            mydir = os.path.dirname(os.path.realpath(__file__))
            self.executable = os.path.join(mydir, '../../../..', 'bin', self.executable)

        # Save the output dir since the current working directory changes during tests
        self.output_dir = os.path.join(os.path.abspath(os.path.dirname(sys.argv[0])), self.options.output_dir)

        # Create the output dir if they ask for it. It is easier to ask for forgiveness than permission
        if self.options.output_dir:
            try:
                os.makedirs(self.output_dir)
            except OSError as ex:
                if ex.errno == errno.EEXIST: pass
                else: raise

        # Use a previous results file, or declare the variable
        self.options.results_storage = {}
        if self.useExistingStorage():
            with open(self.options.results_file, 'r') as f:
                try:
                    self.options.results_storage = json.load(f)

                    # Adhere to previous input file syntax, or set the default
                    _input_file_name = 'tests'
                    if self.options.input_file_name:
                        _input_file_name = self.options.input_file_name
                    self.options.input_file_name = self.options.results_storage.get('INPUT_FILE_NAME', _input_file_name)

                except ValueError:
                    # This is a hidden file, controled by the TestHarness. So we probably shouldn't error
                    # and exit. Perhaps a warning instead, and create a new file? Down the road, when
                    # we use this file for PBS etc, this should probably result in an exception.
                    print(('INFO: Previous %s file is damaged. Creating a new one...' % (self.results_storage)))

    def useExistingStorage(self):
        """ reasons for returning bool if we should use a previous results_storage file """
        if (os.path.exists(self.options.results_file)
            and (self.options.failed_tests or self.options.pbs or self.options.show_last_run)):
            return True
        elif ((self.options.failed_tests or self.options.show_last_run)
            and not os.path.exists(self.options.results_file)):
            print('A previous run does not exist')
            sys.exit(1)


    ## Parse command line options and assign them to self.options
    def parseCLArgs(self, argv):
        parser = argparse.ArgumentParser(description='A tool used to test MOOSE based applications')
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
        parser.add_argument('--longest-jobs', action='store', dest='longest_jobs', type=int, default=0, help='Print the longest running jobs upon completion')
        parser.add_argument('-s', '--scale', action='store_true', dest='scaling', help='Scale problems that have SCALE_REFINE set')
        parser.add_argument('-i', nargs=1, action='store', type=str, dest='input_file_name', default='', help='The test specification file to look for')
        parser.add_argument('--libmesh_dir', nargs=1, action='store', type=str, dest='libmesh_dir', help='Currently only needed for bitten code coverage')
        parser.add_argument('--skip-config-checks', action='store_true', dest='skip_config_checks', help='Skip configuration checks (all tests will run regardless of restrictions)')
        parser.add_argument('--parallel', '-p', nargs='?', action='store', type=int, dest='parallel', const=1, help='Number of processors to use when running mpiexec')
        parser.add_argument('--n-threads', nargs=1, action='store', type=int, dest='nthreads', default=1, help='Number of threads to use when running mpiexec')
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
        parser.add_argument('--spec-file', action='store', type=str, dest='spec_file', help='Supply a path to the tests spec file to run the tests found therein. Or supply a path to a directory in which the TestHarness will search for tests. You can further alter which tests spec files are found through the use of -i and --re')
        parser.add_argument('-C', '--test-root', nargs=1, metavar='dir', type=str, dest='spec_file', help='Tell the TestHarness to search for test spec files at this location.')
        parser.add_argument('-d', '--pedantic-checks', action='store_true', dest='pedantic_checks', help="Run pedantic checks of the Testers' file writes looking for race conditions.")

        # Options that pass straight through to the executable
        parser.add_argument('--parallel-mesh', action='store_true', dest='parallel_mesh', help='Deprecated, use --distributed-mesh instead')
        parser.add_argument('--distributed-mesh', action='store_true', dest='distributed_mesh', help='Pass "--distributed-mesh" to executable')
        parser.add_argument('--error', action='store_true', help='Run the tests with warnings as errors (Pass "--error" to executable)')
        parser.add_argument('--error-unused', action='store_true', help='Run the tests with errors on unused parameters (Pass "--error-unused" to executable)')
        parser.add_argument('--error-deprecated', action='store_true', help='Run the tests with errors on deprecations')
        parser.add_argument('--allow-unused',action='store_true', help='Run the tests without errors on unused parameters (Pass "--allow-unused" to executable)')
        # Option to use for passing unwrapped options to the executable
        parser.add_argument('--cli-args', nargs='?', type=str, dest='cli_args', help='Append the following list of arguments to the command line (Encapsulate the command in quotes)')
        parser.add_argument('--dry-run', action='store_true', dest='dry_run', help="Pass --dry-run to print commands to run, but don't actually run them")
        parser.add_argument('--use-subdir-exe', action="store_true", help='If there are sub directories that contain a new testroot, use that for running tests under that directory.')

        # Options which manipulate the output in some way
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
        outputgroup.add_argument('--include-input-file', action='store_true', dest='include_input', help='Include the contents of the input file when writing the results of a test to a file')
        outputgroup.add_argument("--testharness-unittest", action="store_true", help="Run the TestHarness unittests that test the TestHarness.")
        outputgroup.add_argument("--json", action="store_true", dest="json", help="Dump the parameters for the testers in JSON Format")
        outputgroup.add_argument("--yaml", action="store_true", dest="yaml", help="Dump the parameters for the testers in Yaml Format")
        outputgroup.add_argument("--dump", action="store_true", dest="dump", help="Dump the parameters for the testers in GetPot Format")
        outputgroup.add_argument("--no-trimmed-output", action="store_true", dest="no_trimmed_output", help="Do not trim the output")
        outputgroup.add_argument("--no-trimmed-output-on-error", action="store_true", dest="no_trimmed_output_on_error", help="Do not trim output for tests which cause an error")
        outputgroup.add_argument("--results-file", nargs=1, default=self.results_file, help="Save run_tests results to an alternative json file (default: %(default)s)")
        outputgroup.add_argument("--show-last-run", action="store_true", dest="show_last_run", help="Display previous results without executing tests again")

        queuegroup = parser.add_argument_group('Queue Options', 'Options controlling which queue manager to use')
        queuegroup.add_argument('--pbs', nargs=1, action='store', metavar='name', help='Launch tests using PBS as your scheduler. You must supply a name to identify this session with')
        queuegroup.add_argument('--pbs-pre-source', nargs=1, action="store", dest='queue_source_command', metavar='', help='Source specified file before launching tests')
        queuegroup.add_argument('--pbs-project', nargs=1, action='store', dest='queue_project', type=str, default='moose', metavar='', help='Identify your job(s) with this project (default:  %(default)s)')
        queuegroup.add_argument('--pbs-queue', nargs=1, action='store', dest='queue_queue', type=str, metavar='', help='Submit jobs to the specified queue')
        queuegroup.add_argument('--pbs-node-cpus', nargs=1, action='store', type=int, default=None, metavar='', help='CPUS Per Node. The default (no setting), will always use only one node')
        queuegroup.add_argument('--pbs-cleanup', nargs=1, action="store", dest='queue_cleanup', metavar='name', help='Clean up files generated by supplied --pbs name')

        code = True
        if self.code.decode() in argv:
            del argv[argv.index(self.code.decode())]
            code = False
        self.options = parser.parse_args(argv[1:])
        self.options.code = code

        self.options.runtags = [tag for tag in self.options.run.split(',') if tag != '']

        # Convert all list based options of length one to scalars
        for key, value in list(vars(self.options).items()):
            if type(value) == list and len(value) == 1:
                setattr(self.options, key, value[0])

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
        if opts.valgrind_mode and opts.nthreads > 1:
            print('ERROR: --threads cannot be used with --valgrind')
            sys.exit(1)
        if opts.check_input and opts.no_check_input:
            print('ERROR: --check-input and --no-check-input cannot be used simultaneously')
            sys.exit(1)
        if opts.check_input and opts.enable_recover:
            print('ERROR: --check-input and --recover cannot be used simultaneously')
            sys.exit(1)
        if opts.spec_file and not os.path.exists(opts.spec_file):
            print('ERROR: --spec-file supplied but path does not exist')
            sys.exit(1)
        if opts.queue_cleanup and not opts.pbs:
            print('ERROR: --queue-cleanup cannot be used without additional queue options')
            sys.exit(1)
        if opts.queue_source_command and not os.path.exists(opts.queue_source_command):
            print('ERROR: pre-source supplied but path does not exist')
            sys.exit(1)
        if opts.failed_tests and not opts.pbs and not os.path.exists(opts.results_file):
            print('ERROR: --failed-tests could not detect a previous run')
            sys.exit(1)
        if opts.pbs and opts.pedantic_checks:
            print('ERROR: --pbs and --pedantic-checks cannot be used simultaneously')
            sys.exit(1)
        if opts.pbs and opts.jobs:
            print('ERROR: --pbs and -j|--jobs cannot be used simultaneously')
            sys.exit(1)
        if opts.pbs and opts.extra_info:
            print('ERROR: --pbs and -e (extra info) cannot be used simultaneously')
            sys.exit(1)
        if opts.verbose and opts.quiet:
            print('Do not be an oxymoron with --verbose and --quiet')
            sys.exit(1)

        # Flatten input_file_name from ['tests', 'speedtests'] to just tests if none supplied
        # We can not support running two spec files during one launch into a third party queue manager.
        # This is because Jobs created by spec files, have no way of accessing other jobs created by
        # other spec files. They only know about the jobs a single spec file generates.
        # NOTE: Which means, tests and speedtests running simultaneously currently have a chance to
        # clobber each others output during normal operation!?
        if opts.pbs and not opts.input_file_name:
            self.options.input_file_name = 'tests'

        # Update any keys from the environment as necessary
        if not self.options.method:
            if 'METHOD' in os.environ:
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

        # User wants to write all output, so unify the options involved
        if opts.sep_files:
            opts.ok_files = True
            opts.fail_files = True
            opts.quiet = True

        # User wants only failed files, so unify the options involved
        elif opts.fail_files:
            opts.quiet = True

    def postRun(self, specs, timing):
        return

    def preRun(self):
        if self.options.json:
            self.factory.printJSON("Tests")
            sys.exit(0)
        elif self.options.yaml:
            self.factory.printYaml("Tests")
            sys.exit(0)
        elif self.options.dump:
            self.factory.printDump("Tests")
            sys.exit(0)

    def getOptions(self):
        return self.options
