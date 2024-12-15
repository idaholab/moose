#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import platform
import os, re, inspect, errno, copy, json
from . import RaceChecker
import subprocess
import shutil
import socket
import datetime
import getpass
from collections import namedtuple

from socket import gethostname
from FactorySystem.Factory import Factory
from FactorySystem.Parser import Parser
from FactorySystem.Warehouse import Warehouse
from . import util
import pyhit

import argparse

# Directory the test harness is in
testharness_dir = os.path.dirname(os.path.realpath(__file__))

def readTestRoot(fname):

    root = pyhit.load(fname)
    args = root.get('run_tests_args', '').split()

    # TODO: add check to see if the binary exists before returning. This can be used to
    # allow users to control fallthrough for e.g. individual module binaries vs. the
    # combined binary.
    return root.get('app_name'), args, root

# Struct that represents all of the information pertaining to a testroot file
TestRoot = namedtuple('TestRoot', ['root_dir', 'app_name', 'args', 'hit_node'])
def findTestRoot() -> TestRoot:
    """
    Search for the test root in all folders above this one
    """
    start = os.getcwd()
    root_dir = start
    while os.path.dirname(root_dir) != root_dir:
        testroot_file = os.path.join(root_dir, 'testroot')
        if os.path.exists(testroot_file) and os.access(testroot_file, os.R_OK):
            app_name, args, hit_node = readTestRoot(testroot_file)
            return TestRoot(root_dir=root_dir, app_name=app_name, args=args, hit_node=hit_node)
        root_dir = os.path.dirname(root_dir)
    return None

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
    def buildAndRun(argv: list, app_name: str, moose_dir: str, moose_python: str = None,
                    skip_testroot: bool = False) -> None:
        # Cannot skip the testroot if we don't have an application name
        if skip_testroot and not app_name:
            raise ValueError(f'Must provide "app_name" when skip_testroot=True')

        # Assume python directory from moose (in-tree)
        if moose_python is None:
            moose_python_dir = os.path.join(moose_dir, "python")
        # Given a python directory (installed app)
        else:
            moose_python_dir = moose_python

        # Set MOOSE_DIR and PYTHONPATH for child processes
        os.environ['MOOSE_DIR'] = moose_dir
        pythonpath = os.environ.get('PYTHONPATH', '').split(':')
        if moose_python_dir not in pythonpath:
            pythonpath = [moose_python_dir] + pythonpath
            os.environ['PYTHONPATH'] = ':'.join(pythonpath)

        # Search for the test root (if any; required when app_name is not specified)
        test_root = None if skip_testroot else findTestRoot()

        # Failed to find a test root
        if test_root is None:
            # app_name was specified so without a testroot, we don't
            # know what application to run
            if app_name is None:
                raise RuntimeError(f'Failed to find testroot by traversing upwards from {os.getcwd()}')
            # app_name was specified so just run from this directory
            # without any additional parameters
            test_root = TestRoot(root_dir='.', app_name=app_name,
                                 args=[], hit_node=pyhit.Node())
        # Found a testroot, but without an app_name
        elif test_root.app_name is None:
            # app_name was specified from buildAndRun(), so use it
            if app_name:
                test_root = test_root._replace(app_name=app_name)
            # Missing an app_name
            else:
                raise RuntimeError(f'{test_root.root_dir}/testroot missing app_name')

        harness = TestHarness(argv, moose_dir, moose_python_dir, test_root)
        harness.findAndRunTests()
        sys.exit(harness.error_code)

    def __init__(self, argv: list, moose_dir: str, moose_python: str, test_root: TestRoot):
        self.moose_python_dir = moose_python
        self._rootdir = test_root.root_dir
        self._orig_cwd = os.getcwd()
        os.chdir(test_root.root_dir)
        argv = argv[:1] + test_root.args + argv[1:]

        self.factory = Factory()

        self.app_name = test_root.app_name
        self.root_params = test_root.hit_node

        # Build a Warehouse to hold the MooseObjects
        self.warehouse = Warehouse()

        # Testers from this directory
        dirs = [os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))]

        # Get dependent applications and load dynamic tester plugins
        # If applications have new testers, we expect to find them in <app_dir>/scripts/TestHarness/testers
        # Use the find_dep_apps script to get the dependent applications for an app
        app_dirs = findDepApps(self.app_name, use_current_only=True).split('\n')
        # For installed binaries, the apps will exist in RELEASE_PATH/scripts, where in
        # this case RELEASE_PATH is moose_dir
        share_dir = os.path.join(moose_dir, 'share')
        if os.path.isdir(share_dir):
            for dir in os.listdir(share_dir):
                if dir != 'moose': # already included
                    app_dirs.append(os.path.join(share_dir, dir))
        # Add scripts/TestHarness for all of the above
        dirs.extend([os.path.join(my_dir, 'scripts', 'TestHarness') for my_dir in app_dirs])

        # Finally load the plugins!
        self.factory.loadPlugins(dirs, 'testers', "IS_TESTER")

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
        checks['machine'] = util.getMachine()
        checks['submodules'] = util.getInitializedSubmodules(self.run_tests_dir)
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
            checks['libtorch'] = set(['ALL'])
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

        self.initialize(argv, self.app_name)

        # executable is available after initalize
        checks['installation_type'] = util.checkInstalled(self.executable, self.app_name)

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
        self.start_time = datetime.datetime.now()
        launched_tests = []

        if self.options.spec_file and os.path.isdir(self.options.spec_file):
            search_dir = self.options.spec_file
        elif self.options.spec_file and os.path.isfile(self.options.spec_file):
            search_dir = os.path.dirname(self.options.spec_file)
            assert self.options.input_file_name == os.path.basename(self.options.spec_file)
        else:
            search_dir = os.getcwd()

        try:
            testroot_params = {}
            for dirpath, dirnames, filenames in os.walk(search_dir, followlinks=True):
                # Prune submodule paths when searching for tests, allowing exception
                # for a git submodule contained within the test/tests or tests folder

                dir_name = os.path.basename(dirpath)
                if (search_dir != dirpath and os.path.exists(os.path.join(dirpath, '.git'))) or dir_name in [".git", ".svn"]:
                    cdir = os.path.join(search_dir, 'test/tests/')
                    if (os.path.commonprefix([dirpath, cdir]) == cdir):
                        continue

                    cdir = os.path.join(search_dir, 'tests/')
                    if (os.path.commonprefix([dirpath, cdir]) == cdir):
                        continue

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
                        if file == self.options.input_file_name \
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
        if self.options.input_file_name in relative_path:
            relative_path = relative_path.replace('/' + self.options.input_file_name + '/', ':')
        relative_path = re.sub('^[/:]*', '', relative_path)  # Trim slashes and colons
        relative_hitpath = os.path.join(*params['hit_path'].split(os.sep)[2:])  # Trim root node "[Tests]"
        formatted_name = relative_path + '.' + relative_hitpath

        params['spec_file'] = filename
        params['test_name'] = formatted_name
        params['test_name_short'] = relative_hitpath
        params['test_dir'] = test_dir
        params['executable'] = testroot_params.get("executable", self.executable)
        params['app_name'] = self.app_name
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
                part1.appendTestName('_part1')
                part1_params = part1.parameters()
                part1_params['cli_args'].append('--test-checkpoint-half-transient')
                if self.options.recoversuffix == 'cpa':
                    part1_params['cli_args'].append('Outputs/out/type=Checkpoint')
                    part1_params['cli_args'].append('Outputs/out/binary=false')
                part1_params['skip_checks'] = True

                # Part 2:
                part2_params = part2.parameters()
                part2_params['prereq'].append(part1.getTestNameShort())
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

    def handleJobStatus(self, job, caveats=None):
        """
        The Scheduler is calling back the TestHarness to inform us of a status change.
        The job may or may not be finished yet (RUNNING), or failing, passing, etc.
        """
        if self.options.show_last_run and job.isSkip():
            return
        elif not job.isSilent():
            # Print results and perform any desired post job processing
            if job.isFinished():
                joint_status = job.getJointStatus()
                self.error_code = self.error_code | joint_status.status_code

                # perform printing of application output if so desired
                output = job.getOutputForScreen()
                if output:
                    print(output)

                # Print status with caveats (if caveats not overridden)
                caveats = True if caveats is None else caveats
                print(util.formatResult(job, self.options, caveats=caveats), flush=True)

                timing = job.getTiming()

                # Save these results for 'Final Test Result' summary
                self.test_table.append( (job, joint_status.sort_value, timing) )
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
                caveats = False if caveats is None else caveats
                print(util.formatResult(job, self.options, result=job.getStatus().status, caveats=caveats), flush=True)

    def getStats(self, time_total: float) -> dict:
        """
        Get cumulative stats for all runs
        """
        num_nonzero_timing = sum(1 if float(tup[0].getTiming()) > 0 else 0 for tup in self.test_table)
        if num_nonzero_timing > 0:
            time_max = max(float(tup[0].getTiming()) for tup in self.test_table)
            time_average = sum(float(tup[0].getTiming()) for tup in self.test_table) / num_nonzero_timing
        else:
            time_max = 0
            time_average = 0

        stats = {'num_passed': self.num_passed,
                 'num_failed': self.num_failed,
                 'num_skipped': self.num_skipped,
                 'num_total': self.num_passed + self.num_failed + self.num_skipped,
                 'time_total': time_total,
                 'time_max': time_max,
                 'time_average': time_average}
        stats.update(self.scheduler.appendStats())
        return stats

    # Print final results, close open files, and exit with the correct error code
    def cleanup(self):
        # Print the results table again if a bunch of output was spewed to the screen between
        # tests as they were running
        if len(self.parse_errors) > 0:
            print(('\n\nParser Errors:\n' + ('-' * (self.options.term_cols))))
            for err in self.parse_errors:
                print((util.colorText(err, 'RED', html=True, colored=self.options.colored, code=self.options.code)))

        if (self.options.verbose or (self.num_failed != 0 and not self.options.quiet)) and not self.options.dry_run:
            print(('\n\nFinal Test Results:\n' + ('-' * (self.options.term_cols))))
            for (job, sort_value, timing) in sorted(self.test_table, key=lambda x: x[1]):
                print((util.formatResult(job, self.options, caveats=True)))

        time_total = (datetime.datetime.now() - self.start_time).total_seconds()
        stats = self.getStats(time_total)

        print(('-' * (self.options.term_cols)))

        # Mask off TestHarness error codes to report parser errors
        fatal_error = ''
        if len(self.parse_errors) > 0:
            fatal_error += ', <r>FATAL PARSER ERROR</r>'
            self.error_code = self.error_code | 0x80

        # Print a different footer when performing a dry run
        if self.options.dry_run:
            print(f'Processed {self.num_passed + self.num_skipped} tests in {stats["time_total"]:.1f} seconds.')
            summary = f'<b>{self.num_passed} would run</b>, <b>{self.num_skipped} would be skipped</b>'
            summary += fatal_error
            print(util.colorText(summary, "", html=True, colored=self.options.colored, code=self.options.code))
        else:
            num_nonzero_timing = sum(1 if float(tup[0].getTiming()) > 0 else 0 for tup in self.test_table)
            if num_nonzero_timing > 0:
                timing_max = max(float(tup[0].getTiming()) for tup in self.test_table)
                timing_avg = sum(float(tup[0].getTiming()) for tup in self.test_table) / num_nonzero_timing
            else:
                timing_max = 0
                timing_avg = 0
            summary = f'Ran {self.num_passed + self.num_failed} tests in {stats["time_total"]:.1f} seconds.'
            summary += f' Average test time {timing_avg:.1f} seconds,'
            summary += f' maximum test time {timing_max:.1f} seconds.'
            print(summary)

            # Get additional results from the scheduler
            scheduler_summary = self.scheduler.appendResultFooter(stats)
            if scheduler_summary:
                print(scheduler_summary)

            if self.num_passed:
                summary = f'<g>{self.num_passed} passed</g>'
            else:
                summary = f'<b>{self.num_passed} passed</b>'
            summary += f', <b>{self.num_skipped} skipped</b>'
            if self.num_pending:
                summary += f', <c>{self.num_pending} pending</c>'
            if self.num_failed:
                summary += f', <r>{self.num_failed} FAILED</r>'
            else:
                summary += f', <b>{self.num_failed} failed</b>'
            if self.scheduler.maxFailures():
                self.error_code = self.error_code | 0x80
                summary += '\n<r>MAX FAILURES REACHED</r>'

            summary += fatal_error

            print(util.colorText(summary, "", html=True, colored=self.options.colored, code=self.options.code))

            if self.options.longest_jobs:
                # Sort all jobs by run time
                sorted_tups = sorted(self.test_table, key=lambda tup: float(tup[0].getTiming()), reverse=True)

                print('\n%d longest running jobs:' % self.options.longest_jobs)
                print(('-' * (self.options.term_cols)))

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
                for jobs, dag in self.scheduler.retrieveDAGs():
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
                    dag_table.append([f'{rel_spec_path}{os.path.sep}{self.options.input_file_name}', f'{v:.3f}'])

                sorted_table = sorted(dag_table, key=lambda dag_table: float(dag_table[1]), reverse=True)
                if sorted_table[0:self.options.longest_jobs]:
                    print(f'\n{self.options.longest_jobs} longest running folders:')
                    print(('-' * (self.options.term_cols)))
                    # We can't use util.formatResults, as we are representing a group of testers
                    for group in sorted_table[0:self.options.longest_jobs]:
                        print(str(group[0]).ljust((self.options.term_cols - (len(group[1]) + 4)), ' '), f'[{group[1]}s]')
                    print('\n')

            all_jobs = self.scheduler.retrieveJobs()

            # Gather and print the jobs with race conditions after the jobs are finished
            # and only run when running --pedantic-checks.
            if self.options.pedantic_checks:
                checker = RaceChecker.RaceChecker(all_jobs)
                if checker.findRacePartners():
                    # Print the unique racer conditions and adjust our error code.
                    self.error_code = checker.printUniqueRacerSets()
                else:
                    print("There are no race conditions.")

            if not self.useExistingStorage():
                # Store the results from each job
                for job_group in all_jobs:
                    for job in job_group:
                        job.storeResults(self.scheduler)

                # And write the results, including the stats
                self.writeResults(complete=True, stats=stats)

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

        except IOError:
            print('Permission error while writing results to disc')
            sys.exit(1)
        except:
            print('Error while writing results to disc')
            sys.exit(1)

    def determineScheduler(self):
        if self.options.hpc_host and not self.options.hpc:
            print(f'ERROR: --hpc must be set with --hpc-host for an unknown host')
            sys.exit(1)

        if self.options.hpc == 'pbs':
            return 'RunPBS'
        elif self.options.hpc == 'slurm':
            return 'RunSlurm'
        # The default scheduler plugin
        return 'RunParallel'

    def initializeResults(self):
        """ Initializes the results storage

        If using existing storage, this will load the previous storage.

        If not using existing storage, this will:
        - Delete the previous storage, if any
        - Setup the header for the storage
        - Write the incomplete storage to file
        """
        if self.useExistingStorage():
            if not os.path.exists(self.options.results_file):
                print(f'The previous run {self.options.results_file} does not exist')
                sys.exit(1)
            try:
                with open(self.options.results_file, 'r') as f:
                    self.options.results_storage = json.load(f)
            except:
                print(f'ERROR: Failed to load result {self.options.results_file}')
                raise

            if self.options.results_storage['incomplete']:
                print(f'ERROR: The previous result {self.options.results_file} is incomplete!')
                sys.exit(1)

            # Adhere to previous input file syntax, or set the default
            self.options.input_file_name = self.options.results_storage.get('input_file_name', self.options.input_file_name)

            # Done working with existing storage
            return

        # Remove the old one if it exists
        if os.path.exists(self.options.results_file):
            os.remove(self.options.results_file)

        # Not using previous or previous failed, initialize a new one
        self.options.results_storage = {}
        storage = self.options.results_storage

        # Record the input file name that was used
        storage['input_file_name'] = self.options.input_file_name

        # The test root directory
        storage['root_dir'] = self._rootdir
        # Record that we are using --sep-files
        storage['sep_files'] = self.options.sep_files

        # Record the Scheduler Plugin used
        storage['scheduler'] = self.scheduler.__class__.__name__

        # Record information on the host we can ran on
        storage['hostname'] = socket.gethostname()
        storage['user'] = getpass.getuser()
        storage['testharness_path'] = os.path.abspath(os.path.join(os.path.abspath(__file__), '..'))
        storage['testharness_args'] = sys.argv[1:]
        storage['moose_dir'] = self.moose_dir

        # Record information from apptainer, if any
        apptainer_container = os.environ.get('APPTAINER_CONTAINER')
        if apptainer_container:
            apptainer = {'path': apptainer_container}
            # Information from ApptainerGenerator generated containers
            var_prefix = 'MOOSE_APPTAINER_GENERATOR'
            generator_name = os.environ.get(f'{var_prefix}_NAME')
            if generator_name:
                for suffix in ['LIBRARY', 'NAME', 'TAG', 'VERSION']:
                    apptainer[f'generator_{suffix.lower()}'] = os.environ.get(f'{var_prefix}_{suffix}')
            storage['apptainer'] = apptainer

        # Record when the run began
        storage['time'] = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')

        # Record any additional data from the scheduler
        storage.update(self.scheduler.appendResultFileHeader())

        # Record whether or not the storage is incomplete
        storage['incomplete'] = True

        # Empty storage for the tests
        storage['tests'] = {}

        # Write the headers
        self.writeResults()

    def writeResults(self, complete=False, stats=None):
        """ Forcefully write the current results to file

        Will not do anything if using existing storage.
        """
        # Not writing results
        if self.useExistingStorage():
            raise Exception('Should not write results')

        # Make it as complete (run is done)
        self.options.results_storage['incomplete'] = not complete
        # Store the stats
        self.options.results_storage['stats'] = stats

        # Store to a temporary file so that we always have a working file
        file = self.options.results_file
        file_in_progress = self.options.results_file + '.inprogress'
        try:
            with open(file_in_progress, 'w') as data_file:
                json.dump(self.options.results_storage, data_file, indent=2)
        except UnicodeDecodeError:
            print(f'\nERROR: Unable to write results {file_in_progress} due to unicode decode/encode error')

            # write to a plain file to aid in reproducing error
            with open(file + '.unicode_error' , 'w') as f:
                f.write(self.options.results_storage)

            raise
        except IOError:
            print(f'\nERROR: Unable to write results {file_in_progress} due to permissions')
            raise

        # Replace the file now that it's complete
        try:
            os.replace(file_in_progress, file)
        except:
            print(f'\nERROR: Failed to move in progress results {file_in_progress} to {file}')
            raise

    def initialize(self, argv, app_name):
        # Load the scheduler plugins
        plugin_paths = [os.path.join(self.moose_dir, 'python', 'TestHarness'), os.path.join(self.moose_dir, 'share', 'moose', 'python', 'TestHarness')]
        self.factory.loadPlugins(plugin_paths, 'schedulers', "IS_SCHEDULER")

        scheduler_plugin = self.determineScheduler()

        # Augment the Scheduler params with plugin params
        plugin_params = self.factory.validParams(scheduler_plugin)

        # Set Scheduler specific params based on some provided options.arguments
        plugin_params['max_processes'] = self.options.jobs
        plugin_params['average_load'] = self.options.load

        # Create the scheduler
        self.scheduler = self.factory.create(scheduler_plugin, self, plugin_params)

        # Now that the scheduler is setup, initialize the results storage
        # Save executable-under-test name to self.executable
        exec_suffix = 'Windows' if platform.system() == 'Windows' else ''
        executable = f'{app_name}-{self.options.method}{exec_suffix}'
        self.app_name = app_name

        # Find the app executable
        self.executable = None
        if os.path.isabs(executable):
            self.executable = executable
        else:
            # Directories to search in
            dirs = [self._orig_cwd, os.getcwd(), self._rootdir,
                    os.path.join(testharness_dir, '../../../../bin')]
            dirs = list(dict.fromkeys(dirs)) # remove duplicates
            for dir in dirs:
                path = os.path.join(dir, executable)
                if os.path.exists(path):
                    self.executable = path
                    break
            if self.executable is None and shutil.which(executable):
                self.executable = shutil.which(executable)

        if self.executable is not None:
            self.executable = os.path.normpath(self.executable)

        # Create the output dir if they ask for it. It is easier to ask for forgiveness than permission
        if self.options.output_dir:
            try:
                os.makedirs(self.options.output_dir)
            except OSError as ex:
                if ex.errno == errno.EEXIST: pass
                else: raise

        # Initialize the results storage or load the previous results
        self.initializeResults()

    def useExistingStorage(self):
        """ reasons for returning bool if we should use a previous results_storage file """
        return self.options.failed_tests or self.options.show_last_run

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
        parser.add_argument('-i', nargs=1, action='store', type=str, dest='input_file_name', help='The test specification file to look for (default: tests)')
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
        parser.add_argument('--libtorch-device', action='store', dest='libtorch_device', type=str, choices=['cpu', 'cuda', 'mps'], default='cpu', help='Run libtorch tests with this device')
        parser.add_argument('--error', action='store_true', help='Run the tests with warnings as errors (Pass "--error" to executable)')
        parser.add_argument('--error-unused', action='store_true', help='Run the tests with errors on unused parameters (Pass "--error-unused" to executable)')
        parser.add_argument('--error-deprecated', action='store_true', help='Run the tests with errors on deprecations')
        parser.add_argument('--allow-unused',action='store_true', help='Run the tests without errors on unused parameters (Pass "--allow-unused" to executable)')
        parser.add_argument('--allow-warnings',action='store_true', help='Run the tests with warnings not as errors (Do not pass "--error" to executable)')
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
        outputgroup.add_argument('-x', '--sep-files', action='store_true', dest='sep_files', help='Write the output of each test to a separate file. Only quiet output to terminal.')
        outputgroup.add_argument('--include-input-file', action='store_true', dest='include_input', help='Include the contents of the input file when writing the results of a test to a file')
        outputgroup.add_argument("--testharness-unittest", action="store_true", help="Run the TestHarness unittests that test the TestHarness.")
        outputgroup.add_argument("--json", action="store_true", dest="json", help="Dump the parameters for the testers in JSON Format")
        outputgroup.add_argument("--yaml", action="store_true", dest="yaml", help="Dump the parameters for the testers in Yaml Format")
        outputgroup.add_argument("--dump", action="store_true", dest="dump", help="Dump the parameters for the testers in GetPot Format")
        outputgroup.add_argument("--no-trimmed-output", action="store_true", dest="no_trimmed_output", help="Do not trim the output")
        outputgroup.add_argument("--no-trimmed-output-on-error", action="store_true", dest="no_trimmed_output_on_error", help="Do not trim output for tests which cause an error")
        outputgroup.add_argument("--results-file", nargs=1, default='.previous_test_results.json', help="Save run_tests results to an alternative json file (default: %(default)s)")
        outputgroup.add_argument("--show-last-run", action="store_true", dest="show_last_run", help="Display previous results without executing tests again")

        # Options for HPC execution
        hpcgroup = parser.add_argument_group('HPC Options', 'Options controlling HPC execution')
        hpcgroup.add_argument('--hpc', dest='hpc', action='store', choices=['pbs', 'slurm'], help='Launch tests using a HPC scheduler')
        hpcgroup.add_argument('--hpc-host', nargs='+', action='store', dest='hpc_host', metavar='', help='The host(s) to use for submitting HPC jobs')
        hpcgroup.add_argument('--hpc-pre-source', nargs=1, action="store", dest='hpc_pre_source', metavar='', help='Source specified file before launching HPC tests')
        hpcgroup.add_argument('--hpc-file-timeout', nargs=1, type=int, action='store', dest='hpc_file_timeout', default=300, help='The time in seconds to wait for HPC output')
        hpcgroup.add_argument('--hpc-scatter-procs', nargs=1, type=int, action='store', dest='hpc_scatter_procs', default=None, help='Set to run HPC jobs with scatter placement when the processor count is this or lower')
        hpcgroup.add_argument('--hpc-apptainer-bindpath', nargs=1, action='store', type=str, dest='hpc_apptainer_bindpath', help='Sets the apptainer bindpath for HPC jobs')
        hpcgroup.add_argument('--hpc-apptainer-no-home', action='store_true', dest='hpc_apptainer_no_home', help='Passes --no-home to apptainer for HPC jobs')
        hpcgroup.add_argument('--hpc-project', nargs=1, action='store', dest='hpc_project', type=str, default='moose', metavar='', help='Identify your job(s) with this project (default:  %(default)s)')
        hpcgroup.add_argument('--hpc-no-hold', nargs=1, action='store', type=bool, default=False, dest='hpc_no_hold', help='Do not pre-create hpc jobs to be held')
        hpcgroup.add_argument('--pbs-queue', nargs=1, action='store', dest='hpc_queue', type=str, metavar='', help='Submit jobs to the specified queue')

        # Try to find the terminal size if we can
        # Try/except here because the terminal size could fail w/o a display
        term_cols = None
        try:
            term_cols = os.get_terminal_size().columns * 7/8
        except:
            term_cols = 110
            pass

        # Optionally load in the environment controlled values
        term_cols = int(os.getenv('MOOSE_TERM_COLS', term_cols))
        term_format = os.getenv('MOOSE_TERM_FORMAT', 'njcst')

        # Terminal options
        termgroup = parser.add_argument_group('Terminal Options', 'Options for controlling the formatting of terminal output')
        termgroup.add_argument('--term-cols', dest='term_cols', action='store', type=int, default=term_cols, help='The number columns to use in output')
        termgroup.add_argument('--term-format', dest='term_format', action='store', type=str, default=term_format, help='The formatting to use when outputting job status')

        code = True
        if self.code.decode() in argv:
            del argv[argv.index(self.code.decode())]
            code = False
        self.options = parser.parse_args(argv[1:])
        self.options.code = code

        # Try to guess the --hpc option if --hpc-host is set
        if self.options.hpc_host and not self.options.hpc:
            hpc_host = self.options.hpc_host[0]
            hpc_config = TestHarness.queryHPCCluster(hpc_host)
            if hpc_config is not None:
                self.options.hpc = hpc_config.scheduler
                print(f'INFO: Setting --hpc={self.options.hpc} for known host {hpc_host}')

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
        if opts.spec_file:
            if not os.path.exists(opts.spec_file):
                print('ERROR: --spec-file supplied but path does not exist')
                sys.exit(1)
            if os.path.isfile(opts.spec_file):
                if opts.input_file_name:
                    print('ERROR: Cannot use -i with --spec-file being a file')
                    sys.exit(1)
                self.options.input_file_name = os.path.basename(opts.spec_file)
        if opts.verbose and opts.quiet:
            print('Do not be an oxymoron with --verbose and --quiet')
            sys.exit(1)
        if opts.error and opts.allow_warnings:
            print(f'ERROR: Cannot use --error and --allow-warnings together')
            sys.exit(1)

        # Setup absolute paths and output paths
        if opts.output_dir:
            opts.output_dir = os.path.abspath(opts.output_dir)
            opts.results_file = os.path.join(opts.output_dir, opts.results_file)
        else:
            opts.results_file = os.path.abspath(opts.results_file)

        if opts.failed_tests and not os.path.exists(opts.results_file):
            print('ERROR: --failed-tests could not detect a previous run')
            sys.exit(1)

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

        # Set default
        if not self.options.input_file_name:
            self.options.input_file_name = 'tests'

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

    # Helper tuple for storing information about a cluster
    HPCCluster = namedtuple('HPCCluster', ['scheduler', 'apptainer_modules'])
    # The modules that we want to load when running in a non-moduled
    # container on INL HPC
    inl_modules = ['use.moose', 'moose-dev-container-openmpi/5.0.5_0']
    # Define INL HPC clusters
    hpc_configs = {'sawtooth': HPCCluster(scheduler='pbs',
                                          apptainer_modules=inl_modules),
                   'bitterroot': HPCCluster(scheduler='slurm',
                                            apptainer_modules=inl_modules)}

    @staticmethod
    def queryHPCCluster(hostname: str):
        """
        Attempt to get the HPC cluster configuration given a host

        Args:
            hostname: The HPC system hostname
        Returns:
            HPCCluster: The config, if found, otherwise None
        """
        for host, config in TestHarness.hpc_configs.items():
            if host in hostname:
                return config
        return None
