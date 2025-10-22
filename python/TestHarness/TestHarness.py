#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import platform
import os, re, inspect, errno, copy, json
import subprocess
import shutil
import socket
import datetime
import getpass
import argparse
import typing
from collections import defaultdict, namedtuple, OrderedDict
from typing import Any, Tuple

from socket import gethostname

from FactorySystem.Factory import Factory
from FactorySystem.Parser import Parser
from FactorySystem.Warehouse import Warehouse
from . import util
from . import RaceChecker
import pyhit

# Directory the test harness is in
testharness_dir = os.path.dirname(os.path.realpath(__file__))

def readTestRoot(fname):

    root = pyhit.load(fname)
    args = root.get('run_tests_args', '').split()

    # TODO: add check to see if the binary exists before returning. This can be used to
    # allow users to control fallthrough for e.g. individual module binaries vs. the
    # combined binary.
    app_name = root.get('app_name') or None

    # Append to PYTHONPATH based on argument in file
    extra_pythonpath_val: str = root.get('extra_pythonpath', None)
    extra_pythonpath_val = extra_pythonpath_val.split(':') if extra_pythonpath_val else []
    extra_pythonpath = []
    for val in extra_pythonpath_val:
        path = os.path.abspath(os.path.join(os.path.dirname(fname), val))
        if not os.path.exists(path):
            raise FileNotFoundError(
                "Test Root Parsing Error: "
                f"Could not find {path} for PYTHONPATH append. "
                f"Check 'extra_pythonpath' in {fname} to resolve."
            )
        else:
            extra_pythonpath.append(path)

    return app_name, args, root, extra_pythonpath

# Struct that represents all of the information pertaining to a testroot file
TestRoot = namedtuple('TestRoot', ['root_dir', 'app_name', 'args', 'hit_node', 'extra_pythonpath'])
def findTestRoot() -> TestRoot:
    """
    Search for the test root in all folders above this one
    """
    start = os.getcwd()
    root_dir = start
    while os.path.dirname(root_dir) != root_dir:
        testroot_file = os.path.join(root_dir, 'testroot')
        if os.path.exists(testroot_file) and os.access(testroot_file, os.R_OK):
            tuple_args = readTestRoot(testroot_file)
            return TestRoot(root_dir, *tuple_args)
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
    p = subprocess.run(
        ['git', 'rev-parse', '--show-cdup'],
        text=True,
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
    )
    if p.returncode == 0:
        git_dir = p.stdout
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
    return '\n'.join(dep_dirs)

class TestHarness:
    # Version history:
    # 1 - Initial tracking of version
    # 2 - Added 'unique_test_id' (tests/*/tests/*/unique_test_id) to Job output if set
    # 3 - Added 'json_metadata' (tests/*/tests/*/tester/json_metadata) to Tester output
    # 4 - Added 'validation' (tests/*/tests/validation) to Job output if set
    # 5 - Added validation data types (tests/*/tests/data/type) to Job output if set
    # 6 - Added 'testharness/validation_version'
    # 7 - Moved test output files from test/*/tests/*/tester/output_files to
    #     job output in test/*/tests/*/output_files
    # 8 - Store tests/*/tests/*/tester/json_metadata values a dict instead of a file path
    RESULTS_VERSION = 8

    # Validation version history:
    # 1 - Initial tracking of version
    # 2 - Added 'abs_zero' key to ValidationNumericData
    VALIDATION_VERSION = 2

    @staticmethod
    def build(argv: list, app_name: str, moose_dir: str, moose_python: str = None,
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

        # Append PYTHONPATH paths specified from testroot
        if test_root:
            for path in test_root.extra_pythonpath:
                if path not in sys.path: # Prevents duplication
                    sys.path.append(path)

        # Failed to find a test root
        if test_root is None:
            # app_name was specified so without a testroot, we don't
            # know what application to run
            if app_name is None:
                raise RuntimeError(f'Failed to find testroot by traversing upwards from {os.getcwd()}')
            # app_name was specified so just run from this directory
            # without any additional parameters
            test_root = TestRoot(root_dir='.', app_name=app_name,
                                 args=[], hit_node=pyhit.Node(), extra_pythonpath="")
        # Found a testroot, but without an app_name
        elif test_root.app_name is None:
            # app_name was specified from buildAndRun(), so use it
            if app_name:
                test_root = test_root._replace(app_name=app_name)
            # Missing an app_name
            elif app_name is not None:
                raise RuntimeError(f'{test_root.root_dir}/testroot missing app_name')

        return TestHarness(argv, moose_dir, moose_python_dir, test_root)

    @staticmethod
    def buildAndRun(argv: list, app_name: str, moose_dir: str, *args, **kwargs) -> None:
        harness = TestHarness.build(argv, app_name, moose_dir, *args, **kwargs)
        harness.findAndRunTests()
        sys.exit(harness.error_code)

    @staticmethod
    def validComputeDevices():
        return ['cpu', 'cuda', 'hip', 'mps', 'ceed-cpu', 'ceed-cuda', 'ceed-hip']

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
        if self.app_name:
            app_dirs = findDepApps(self.app_name, use_current_only=True).split('\n')
        else:
            app_dirs = []
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
        self.finished_jobs: list = []
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

        # Failed Tests file object
        self.writeFailedTest = None

        # Parse arguments
        self.parseCLArgs(argv)

        # Determine the executable if we have an application
        try:
            self.executable = self.getExecutable() if self.app_name else None
        except FileNotFoundError as e:
            self.errorExit(f'{e}')

        # If we have an executable and there is python directory next to it,
        # append that directory to PYTHONPATH
        if self.executable is not None:
            exe_python_dir = os.path.join(os.path.dirname(os.path.abspath(self.executable)), 'python')
            if os.path.isdir(exe_python_dir) and exe_python_dir not in sys.path:
                sys.path.append(exe_python_dir)

        # Load capabilities if they're needed
        self.options._required_capabilities = []
        if self.options.no_capabilities:
            self.options._capabilities = None
            if self.options.only_tests_that_require:
                self.errorExit('Cannot use --only-tests-that-require with --no-capabilities')
        else:
            assert self.executable

            # Make sure capabilities are available early; this will exit
            # if we fail
            import pycapabilities

            with util.ScopedTimer(0.5, 'Parsing application capabilities'):
                self.options._capabilities = util.getCapabilities(self.executable)

            # Setup the required capabilities, if any. From the capabilities
            # given by the user, we form the value that we should set them to
            # when temporarily augmenting the capabilities in a Tester
            # to perform a check to see if the capability check in the tester
            # changes if we change these value(s)
            if self.options.only_tests_that_require:
                required = self.options.only_tests_that_require
                if isinstance(required, str):
                    required = [required]
                self.options._required_capabilities = self.buildRequiredCapabilities(
                    list(self.options._capabilities.keys()),
                    required
                )

        checks = {}
        checks['platform'] = util.getPlatforms()
        checks['machine'] = util.getMachine()
        checks['submodules'] = util.getInitializedSubmodules(self.run_tests_dir)
        checks['exe_objects'] = None # This gets calculated on demand
        checks['registered_apps'] = None # This gets extracted on demand

        # The TestHarness doesn't strictly require the existence of an app
        # in order to run. Here we allow the user to select whether they
        # want to probe for configuration options
        if self.options.no_capabilities:
            checks['compiler'] = set(['ALL'])
            for prefix in ['petsc', 'slepc', 'vtk', 'libtorch', 'mfem', 'kokkos']:
                checks[f'{prefix}_version'] = 'N/A'
            for var in ['library_mode', 'mesh_mode', 'unique_ids', 'vtk',
                        'tecplot', 'dof_id_bytes', 'petsc_debug', 'curl',
                        'threading', 'superlu', 'mumps', 'strumpack',
                        'parmetis', 'chaco', 'party', 'ptscotch',
                        'slepc', 'unique_id', 'boost', 'fparser_jit',
                        'libpng', 'libtorch', 'libtorch_version',
                        'installation_type', 'mfem', 'kokkos']:
                checks[var] = set(['ALL'])
        else:
            def get_option(*args, **kwargs):
                return util.getCapabilityOption(self.options._capabilities, *args, **kwargs)

            checks['compiler'] = get_option('compiler', to_set=True)
            checks['petsc_version'] = get_option('petsc', from_version=True)
            checks['petsc_debug'] = get_option('petsc_debug', from_type=bool, to_set=True)
            checks['slepc_version'] = get_option('slepc', from_version=True)
            checks['slepc'] = get_option('slepc', from_version=True, to_set=True, to_bool=True)
            checks['exodus_version'] = get_option('exodus', from_version=True)
            checks['vtk_version'] = get_option('vtk', from_version=True)
            checks['vtk'] = get_option('vtk', from_version=True, to_set=True, to_bool=True)
            checks['library_mode'] = util.getSharedOption(self.libmesh_dir)
            checks['mesh_mode'] = get_option('mesh_mode', from_type=str, to_set=True)
            checks['unique_ids'] = get_option('unique_id', from_type=bool, to_set=True)
            checks['unique_id'] = checks['unique_ids']
            checks['tecplot'] = get_option('tecplot', from_type=bool, to_set=True)
            checks['dof_id_bytes'] = get_option('dof_id_bytes', from_type=int, to_set=True, no_all=True)

            threading = None
            threads = get_option('threads', from_type=bool)
            if threads:
                threading = next((name for name in ['tbb', 'openmp'] if get_option(name, from_type=bool)), 'pthreads')
            checks['threading'] = set(sorted(['ALL', str(threading).upper()]))

            for name in ['superlu', 'mumps', 'strumpack', 'parmetis', 'chaco', 'party',
                         'ptscotch', 'boost', 'curl', 'mfem', 'kokkos']:
                checks[name] = get_option(name, from_type=bool, to_set=True)

            checks['libpng'] = get_option('libpng', from_type=bool, to_set=True)

            fparser = get_option('fparser')
            checks['fparser_jit'] = set(sorted(['ALL', 'TRUE' if fparser == 'jit' else 'FALSE']))

            checks['libtorch'] = get_option('libtorch', from_version=True, to_set=True, to_bool=True)
            checks['libtorch_version'] = get_option('libtorch', from_version=True, to_none=True)

        # Override the MESH_MODE option if using the '--distributed-mesh'
        # or (deprecated) '--parallel-mesh' option.
        if self.options.distributed_mesh or not self.options.cli_args is None and \
               self.options.cli_args.find('--distributed-mesh') != -1:

            option_set = set(['ALL', 'DISTRIBUTED'])
            checks['mesh_mode'] = option_set

        method = set(['ALL', self.options.method.upper()])
        checks['method'] = method

        # This is so we can easily pass checks around to any scheduler plugin
        self.options._checks = checks

        # Initialize the scheduler
        self.initialize()

        # executable is available after initialize
        if not self.options.no_capabilities:
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

            with util.ScopedTimer(0.5, f'Parsing tests in {search_dir}'):
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
                                app_name, args, root_params, _ = readTestRoot(os.path.join(dirpath, file))
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
    # abspath to basename (dirpath), and the test file in question (file)
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
                self.errorExit("Option 'prereq' needs to be of type list in " + params['test_name'])

        # Double the alloted time for tests when running with the valgrind option
        tester.setValgrindMode(self.options.valgrind_mode)

        # When running in valgrind mode, we end up with a ton of output for each failed
        # test.  Therefore, we limit the number of fails...
        if self.options.valgrind_mode and self.num_failed > self.options.valgrind_max_fails:
            tester.setStatus(tester.fail, 'Max Fails Exceeded')
        elif self.num_failed > self.options.max_fails:
            tester.setStatus(tester.fail, 'Max Fails Exceeded')
        elif tester.parameters().isValid('_have_parse_errors') and tester.parameters()['_have_parse_errors']:
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
                print(util.formatJobResult(job, self.options, caveats=caveats), flush=True)

                # Store job as finished for printing
                self.finished_jobs.append(job)

                if job.isSkip():
                    self.num_skipped += 1
                elif job.isPass():
                    self.num_passed += 1
                elif job.isFail():
                    self.num_failed += 1
                else:
                    self.num_pending += 1
            # Just print current status without a status message
            else:
                caveats = False if caveats is None else caveats
                print(util.formatJobResult(job, self.options, status_message=False, caveats=caveats), flush=True)

    def getStats(self, time_total: float) -> dict:
        """
        Get cumulative stats for all runs
        """
        num_nonzero_timing = sum(1 if job.getTiming() > 0 else 0 for job in self.finished_jobs)
        if num_nonzero_timing > 0:
            time_max = max(job.getTiming() for job in self.finished_jobs)
            time_average = sum(job.getTiming() for job in self.finished_jobs) / num_nonzero_timing
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

    def getLongestJobs(self, num: int) -> list:
        """
        Get the longest running jobs after running all jobs
        """
        jobs = [j for j in self.finished_jobs if (not j.isSkip() and j.getTiming() > 0)]
        jobs = sorted(jobs, key=lambda job: job.getTiming(), reverse=True)
        return jobs[0:num]

    def getLongestFolders(self, num: int) -> list[typing.Tuple[str, float]]:
        """
        Get the longest running folders after running all jobs
        """
        # Build a mapping for each folder -> jobs in that folder for jobs that ran
        folder_jobs = defaultdict(list)
        for job in [j for j in self.finished_jobs if not j.isSkip()]:
            folder_jobs[job.getTestDir()].append(job)

        folder_times = []
        for folder, jobs in folder_jobs.items():
            # Find the (start, end) time intervals for each job
            intervals = []
            for job in jobs:
                timer = job.timer
                if job.timer.hasTotalTime('runner_run'):
                    time = timer.getTime('runner_run')
                elif job.timer.hasTotalTime('main'):
                    time = timer.getTime('main')
                else:
                    continue
                intervals.append((time.start, time.end))

            # We have no timing intervals for this folder
            if not intervals:
                continue

            # Find the union of all times spent in this folder;
            # this gives us the total time we ran tests in this folder,
            # where multiple tests running at the same time in the same
            # folder do not count twice
            intervals.sort(key=lambda x: x[0])
            merged_intervals = [intervals[0]]
            for current in intervals:
                last = merged_intervals[-1]
                if current[0] <= last[1]:
                    merged_intervals[-1] = (last[0], max(last[1], current[1]))
                else:
                    merged_intervals.append(current)

            total_time = sum(v[1] - v[0] for v in merged_intervals)
            folder_times.append((os.path.relpath(folder), total_time))

        return sorted(folder_times, key=lambda v: v[1], reverse=True)[0:num]

    # Print final results, close open files, and exit with the correct error code
    def cleanup(self):
        # Helper for printing a header
        def header(title: typing.Optional[str] = None) -> str:
            return (f'\n{title}:\n' if title else '') + '-' * self.options.term_cols

        if not self.finished_jobs:
            print('No tests ran')

        # If something failed or verbosity is requested, print combined results
        if (self.options.verbose or (self.num_failed != 0 and not self.options.quiet)) and not self.options.dry_run:
            print(header('Final Test Results'))
            sorted_jobs = sorted(self.finished_jobs, key=lambda job: job.getJointStatus().sort_value)
            for job in sorted_jobs:
                print((util.formatJobResult(job, self.options, caveats=True)))

        # Longest jobs and longest folders
        if not self.options.dry_run and self.options.longest_jobs:
            longest_jobs = self.getLongestJobs(self.options.longest_jobs)
            if longest_jobs:
                print(header(f'{self.options.longest_jobs} Longest Running Jobs'))
                for job in longest_jobs:
                    print(util.formatJobResult(job, self.options, caveats=True, timing=True))

            longest_folders = self.getLongestFolders(self.options.longest_jobs)
            if longest_folders:
                print(header(f'{self.options.longest_jobs} Longest Running Folders'))
                for folder, time in longest_folders:
                    if self.options.colored and self.options.color_first_directory:
                        first_directory = folder.split('/')[0]
                        prefix = util.colorText(first_directory, 'CYAN')
                        suffix = folder.replace(first_directory, '', 1)
                        folder = prefix + suffix
                    entry = util.FormatResultEntry(name=folder, timing=time)
                    print(util.formatResult(entry, self.options, timing=True))

        # Parser errors, near the bottom
        if self.parse_errors:
            self.error_code = self.error_code | 0x80
            print(header('Parser Errors'))
            for err in self.parse_errors:
                print(err)

        time_total = (datetime.datetime.now() - self.start_time).total_seconds()
        stats = self.getStats(time_total)

        # Final summary for the bottom
        summary = ''
        if self.options.dry_run:
            summary += f'Processed {self.num_passed + self.num_skipped} tests in {stats["time_total"]:.1f} seconds.\n'
            summary += f'<b>{self.num_passed} would run</b>, <b>{self.num_skipped} would be skipped</b>'
        else:
            summary += f'Ran {self.num_passed + self.num_failed} tests in {stats["time_total"]:.1f} seconds.'
            summary += f' Average test time {stats["time_average"]:.1f} seconds,'
            summary += f' maximum test time {stats["time_max"]:.1f} seconds.\n'

            # Get additional results from the scheduler
            scheduler_summary = self.scheduler.appendResultFooter(stats)
            if scheduler_summary:
                summary += scheduler_summary + '\n'

            if self.num_passed:
                summary += f'<g>{self.num_passed} passed</g>'
            else:
                summary += f'<b>{self.num_passed} passed</b>'
            summary += f', <b>{self.num_skipped} skipped</b>'
            if self.num_failed:
                summary += f', <r>{self.num_failed} FAILED</r>'
            else:
                summary += f', <b>{self.num_failed} failed</b>'
            if self.scheduler.maxFailures():
                self.error_code = self.error_code | 0x80
                summary += '\n<r>MAX FAILURES REACHED</r>'
        if self.parse_errors:
            summary += ', <r>FATAL PARSER ERROR</r>'
        print('\n' + header())
        print(util.colorText(summary, "", html=True, colored=self.options.colored))

        if not self.options.dry_run:
            all_jobs = self.scheduler.retrieveJobs()

            # Gather and print the jobs with race conditions after the jobs are finished
            # and only run when running --pedantic-checks.
            if self.options.pedantic_checks:
                checker = RaceChecker.RaceChecker(all_jobs)
                if checker.findRacePartners():
                    # Print the unique racer conditions and adjust our error code.
                    self.error_code = checker.printUniqueRacerSets()

            if not self.useExistingStorage():
                # Store the results from each job
                for job_group in all_jobs:
                    for job in job_group:
                        if not job.isSilent():
                            job.storeResults(self.scheduler)

                # And write the results, including the stats
                self.writeResults(complete=True, stats=stats)

    def determineScheduler(self):
        if self.options.hpc_host and not self.options.hpc:
            self.errorExit('--hpc must be set with --hpc-host for an unknown host')

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
        file = self.options.results_file

        if self.useExistingStorage():
            if not os.path.exists(file):
                self.errorExit(f'The previous run {file} does not exist')
            try:
                with open(file, 'r') as f:
                    results = json.load(f)
            except:
                print(f'ERROR: Failed to load result {file}')
                raise

            testharness = results.get('testharness')
            if testharness is None:
                self.errorExit(f'The previous result {file} is not valid!')

            if not testharness.get('end_time'):
                self.errorExit(f'The previous result {file} is incomplete!')

            # Adhere to previous input file syntax, or set the default
            self.options.input_file_name = testharness.get('input_file_name', self.options.input_file_name)

            # Done working with existing storage
            self.options.results_storage = results
            return

        # Remove the old one if it exists
        if os.path.exists(file):
            os.remove(file)

        # Not using previous or previous failed, initialize a new one
        self.options.results_storage = {}
        storage = self.options.results_storage

        testharness = {'version': self.RESULTS_VERSION,
                       'validation_version': self.VALIDATION_VERSION,
                       'start_time': datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                       'end_time': None,
                       'args': sys.argv[1:],
                       'input_file_name': self.options.input_file_name,
                       'root_dir': self._rootdir,
                       'sep_files': self.options.sep_files,
                       'scheduler': self.scheduler.__class__.__name__,
                       'moose_dir': self.moose_dir}
        storage['testharness'] = testharness

        environment = {'hostname': socket.gethostname(),
                       'user': getpass.getuser()}
        storage['environment'] = environment

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

        # Record any additional data from the scheduler
        storage.update(self.scheduler.appendResultFileHeader())

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

        storage = self.options.results_storage

        # Make it as complete (run is done)
        if complete:
            now = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            storage['testharness']['end_time'] = now

        # Store the stats
        storage['stats'] = stats

        # Store to a temporary file so that we always have a working file
        file = self.options.results_file
        file_in_progress = self.options.results_file + '.inprogress'
        try:
            with open(file_in_progress, 'w') as data_file:
                json.dump(storage, data_file, indent=2)
        except UnicodeDecodeError:
            print(f'\nERROR: Unable to write results {file_in_progress} due to unicode decode/encode error')

            # write to a plain file to aid in reproducing error
            with open(file + '.unicode_error' , 'w') as f:
                f.write(storage)

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

    def getExecutable(self) -> str:
        """
        Finds the MOOSE executable based on the app name
        """
        exec_suffix = 'Windows' if platform.system() == 'Windows' else ''
        name = f'{self.app_name}-{self.options.method}{exec_suffix}'

        # Build list of names for other methods in case an executable exists for
        # a method other than self.options.method
        all_methods = ['opt', 'oprof', 'dbg', 'devel']
        all_names = [f'{self.app_name}-{method}{exec_suffix}' for method in all_methods]

        # Directories to search in
        dirs = [self._orig_cwd, os.getcwd(), self._rootdir,
                os.path.join(testharness_dir, '../../../../bin')]
        dirs = list(dict.fromkeys(dirs)) # remove duplicates
        matches = []
        matched_names = []
        for other_name in all_names:
            for dir in dirs:
                path = os.path.join(dir, other_name)
                if os.path.exists(path):
                    matches.append(path)
                    matched_names.append(other_name)
            exe_path = shutil.which(other_name)
            if exe_path:
                matches.append(exe_path)
                matched_names.append(other_name)

        if name in matched_names:
            return matches[matched_names.index(name)]
        elif len(matched_names):
            # Eliminate any duplicates
            matched_names = set(matched_names)
            available_methods = "'" + "', '".join([matched_name.split('-')[-1] for matched_name in matched_names]) + "'"
            matched_names = "'" + "', '".join(matched_names) + "'"
            err_message = (f'\nThe following executable(s) were found, but METHOD '
                           f'is set to \'{self.options.method}\': {matched_names}'
                           f'\nTo use one of these executables, set the \'METHOD\' environment '
                           f'variable to one of the following values: {available_methods}'
                           )
        else:
            err_message = ""

        raise FileNotFoundError(f'Failed to find MOOSE executable \'{name}\'{err_message}')

    def initialize(self):
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
        term_cols = None
        try:
            term_cols = os.get_terminal_size().columns * 7/8
        except:
            term_cols = 110
            pass
        term_cols = int(os.getenv('MOOSE_TERM_COLS', term_cols))
        term_format = os.getenv('MOOSE_TERM_FORMAT', 'njcst')

        parser = argparse.ArgumentParser(description='A tool used to test MOOSE-based applications')

        parser.add_argument('--failed-tests', action='store_true', help='Run tests that previously failed')
        parser.add_argument('--show-last-run', action='store_true', help='Display previous results without executing tests again')
        parser.add_argument('--dry-run', action='store_true', help="Print the commands to run without running them")

        inputgroup = parser.add_argument_group('Test Specifications', 'Specify which test specification files to load')
        inputgroup.add_argument('-i', nargs=1, action='store', type=str, dest='input_file_name', help='The test specification file to look for (default: tests)')
        inputgroup.add_argument('-C', '--test-root', nargs=1, metavar='dir', type=str, dest='spec_file', help='Search for test spec files in this location')
        inputgroup.add_argument('--spec-file', action='store', type=str,
                                help='Supply a path to the tests spec file to run the tests found therein or supply a path to a directory in which the TestHarness will search for tests')

        parallelgroup = parser.add_argument_group('Parallelization', 'Control the parallel execution')
        parallelgroup.add_argument('-j', '--jobs', nargs='?', action='store', type=int, dest='jobs', const=1, help='Set the number of parallel jobs for tests')
        parallelgroup.add_argument('-l', '--load-average', action='store', type=float, dest='load', help='Do not run additional tests if the load average is at least LOAD')
        parallelgroup.add_argument('-p', '--parallel', nargs='?', action='store', type=int, dest='parallel', const=1, help='Number of MPI processes to use for each job')
        parallelgroup.add_argument('--n-threads', nargs=1, action='store', type=int, dest='nthreads', default=1, help='Number of threads to use when running mpiexec')

        filtergroup = parser.add_argument_group('Test Filters', 'Filter which tests are ran')
        filtergroup.add_argument('-g', '--group', action='store', type=str, dest='group', default='ALL', help='Run only tests in the named group')
        filtergroup.add_argument('-s', '--scale', action='store_true', dest='scaling', help='Run tests that have SCALE_REFINE set')
        filtergroup.add_argument('--all-tests', action='store_true', help='Run heavy and non-heavy tests')
        filtergroup.add_argument('--check-input', action='store_true', help='Run check_input (syntax) tests only')
        filtergroup.add_argument('--heavy', action='store_true', dest='heavy_tests', help='Run tests marked with heavy')
        filtergroup.add_argument('--ignore', nargs='?', action='store', metavar='caveat', dest='ignored_caveats', const='all', type=str,
                                 help='Ignore specified caveats when checking if a test should run; using --ignore without a conditional will ignore all caveats')
        filtergroup.add_argument('--no-check-input', action='store_true', help='Do not run check_input (syntax) tests')
        filtergroup.add_argument('--not-group', action='store', type=str, help='Run only tests NOT in the named group')
        filtergroup.add_argument('--re', action='store', type=str, dest='reg_exp', help='Run tests that match the given regular expression')
        filtergroup.add_argument('--only-tests-that-require', action='extend', nargs=1, type=str, help='Require that a test depend on this capability name; can be negated with "!"')
        filtergroup.add_argument('--valgrind', action='store_const', dest='valgrind_mode', const='NORMAL', help='Run normal valgrind tests')
        filtergroup.add_argument('--valgrind-heavy', action='store_const', dest='valgrind_mode', const='HEAVY', help='Run heavy valgrind tests')

        capabilitygroup = parser.add_argument_group('Additional Capabilities', 'Enable or disable additional TestHarness capabilities')
        capabilitygroup.add_argument('--capture-perf-graph', action='store_true', help='Capture PerfGraph for RunApp tests via Outputs/perf_graph_json_file')
        capabilitygroup.add_argument('--cli-args', nargs='?', type=str, help='Append the following list of arguments to the command line (encapsulate the command in quotes)')
        capabilitygroup.add_argument('--no-capabilities', action='store_true', help='Disable Capability checks')
        capabilitygroup.add_argument('--pedantic-checks', action='store_true', help='Run pedantic checks of the Testers\' file writes looking for race conditions')
        capabilitygroup.add_argument('--use-subdir-exe', action="store_true", help='If there are sub directories that contain a new testroot, use that for running tests under that directory')
        capabilitygroup.add_argument('--recover', action='store_true', dest='enable_recover', help='Run tests in recover mode')
        capabilitygroup.add_argument('--restep', action='store_true', dest='enable_restep', help='Run tests in restep mode')

        appgroup = parser.add_argument_group('Application Options', 'Options that pass arguments directly to the executable')
        appgroup.add_argument('--allow-unused',action='store_true', help='Run the tests without errors on unused parameters (pass --allow-unused)')
        appgroup.add_argument('--allow-warnings',action='store_true', help='Run the tests with warnings not as errors (do not pass --error)')
        appgroup.add_argument('--compute-device', action='store', type=str, choices=TestHarness.validComputeDevices(), default='cpu',
                              help='Run tests that support this compute device; (passes --compute-device=...)')
        appgroup.add_argument('--distributed-mesh', action='store_true', help='Run tests that support distributed mesh (pass --distributed-mesh)')
        appgroup.add_argument('--error', action='store_true', help='Run the tests with warnings as errors (pass --error)')
        appgroup.add_argument('--error-unused', action='store_true', help='Run the tests with errors on unused parameters (pass --error-unused)')
        appgroup.add_argument('--error-deprecated', action='store_true', help='Run the tests with errors on deprecations (pass --error-deprecated)')
        appgroup.add_argument('--recoversuffix', action='store', type=str, default='cpr', help='Set the file suffix for recover mode (pass --recoversuffix)')

        methodgroup = parser.add_argument_group('Application Methods', 'Control which application biunary method is to be ran')
        methodgroup.add_argument('--opt', action='store_const', dest='method', const='opt', help='Test the <app_name>-opt binary')
        methodgroup.add_argument('--dbg', action='store_const', dest='method', const='dbg', help='Test the <app_name>-dbg binary')
        methodgroup.add_argument('--devel', action='store_const', dest='method', const='devel', help='Test the <app_name>-devel binary')
        methodgroup.add_argument('--oprof', action='store_const', dest='method', const='oprof', help='Test the <app_name>-oprof binary')

        screengroup = parser.add_argument_group('On-screen Output', 'Control the on-screen output')
        screengroup.add_argument('-c', '--no-color', action='store_false', dest='colored', help='Do not show colored output')
        screengroup.add_argument('-e', action='store_true', dest='extra_info', help='Display "extra" information including all caveats and deleted tests')
        screengroup.add_argument('-q', '--quiet', action='store_true', dest='quiet', help='Only show the result of every test (even failed output)')
        screengroup.add_argument('-t', '--timing', action='store_true', dest='timing', help='Report Timing information for passing tests')
        screengroup.add_argument('-v', '--verbose', action='store_true', dest='verbose', help='Show the output of every test')
        screengroup.add_argument('--color-first-directory', action='store_true', help='Color first directory')
        screengroup.add_argument('--longest-jobs', action='store', type=int, default=0, help='Print the longest running jobs upon completion')
        screengroup.add_argument('--no-report', action='store_false', dest='report_skipped', help='Do not report skipped tests')
        screengroup.add_argument("--no-trimmed-output", action="store_true", help="Do not trim the output")
        screengroup.add_argument("--no-trimmed-output-on-error", action="store_true", help="Do not trim output for tests which cause an error")
        screengroup.add_argument('--term-cols', action='store', type=int, default=term_cols, help='The number columns to use in output')
        screengroup.add_argument('--term-format', action='store', type=str, default=term_format, help='The formatting to use when outputting job status')

        outputgroup = parser.add_argument_group('Output', 'Control the file output')
        outputgroup.add_argument('-o', '--output-dir', nargs=1, metavar='directory', dest='output_dir', default='', help='Save all output files in the directory, and create it if necessary')
        outputgroup.add_argument('-x', '--sep-files', action='store_true', dest='sep_files', help='Write the output of each test to a separate file. Only quiet output to terminal.')
        outputgroup.add_argument("--results-file", nargs=1, default='.previous_test_results.json', help="Save run_tests results to an alternative json file (default: %(default)s)")

        failgroup = parser.add_argument_group('Failure Criteria', 'Control the failure criteria')
        failgroup.add_argument('--max-fails', nargs=1, type=int, default=50, help='The number of tests allowed to fail before any additional tests will run')
        failgroup.add_argument('--valgrind-max-fails', nargs=1, type=int, default=5, help='The number of valgrind tests allowed to fail before any additional valgrind tests will run')

        hpcgroup = parser.add_argument_group('HPC', 'Enable and control HPC execution')
        hpcgroup.add_argument('--hpc', dest='hpc', action='store', choices=['pbs', 'slurm'], help='Launch tests using a HPC scheduler')
        hpcgroup.add_argument('--hpc-apptainer-bindpath', nargs=1, action='store', type=str, help='Sets the apptainer bindpath for HPC jobs')
        hpcgroup.add_argument('--hpc-apptainer-no-home', action='store_true',  help='Passes --no-home to apptainer for HPC jobs')
        hpcgroup.add_argument('--hpc-file-timeout', nargs=1, type=int, action='store', default=300, help='The time in seconds to wait for HPC output')
        hpcgroup.add_argument('--hpc-host', nargs='+', action='store', metavar='', help='The host(s) to use for submitting HPC jobs')
        hpcgroup.add_argument('--hpc-no-hold', nargs=1, action='store', type=bool, default=False, help='Do not pre-create hpc jobs to be held')
        hpcgroup.add_argument('--hpc-pre-source', nargs=1, action="store", metavar='', help='Source specified file before launching HPC tests')
        hpcgroup.add_argument('--hpc-project', nargs=1, action='store',  type=str, default='moose', metavar='', help='Identify your job(s) with this project (default:  %(default)s)')
        hpcgroup.add_argument('--hpc-scatter-procs', nargs=1, type=int, action='store', dest='hpc_scatter_procs', default=None, help='Set to run HPC jobs with scatter placement when the processor count is this or lower')
        hpcgroup.add_argument('--pbs-queue', nargs=1, action='store', dest='hpc_queue', type=str, metavar='', help='Submit jobs to the specified queue')

        dumpgroup = parser.add_argument_group('Syntax Dumping', 'Dump the Tester parameters')
        dumpgroup.add_argument('--json', action='store_true', help='Dump Tester parameters in JSON Format')
        dumpgroup.add_argument('--yaml', action='store_true', help='Dump Tester parameters in Yaml Format')
        dumpgroup.add_argument('--dump', action='store_true', help='Dump Tester parameters in HIT Format')

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
            self.errorExit('The group and not_group options cannot specify the same group')
        if opts.valgrind_mode and opts.nthreads > 1:
            self.errorExit('--threads cannot be used with --valgrind')
        if opts.check_input and opts.no_check_input:
            self.errorExit('--check-input and --no-check-input cannot be used simultaneously')
        has_flags = []
        for var, flag in [('check_input', '--check-input'),
                          ('enable_recover', '--recover'),
                          ('enable_restep', '--restep')]:
            if getattr(opts, var):
                has_flags.append(flag)
        if len(has_flags) > 1:
            self.errorExit(' and '.join(has_flags), 'cannot be used together')
        if opts.spec_file:
            if not os.path.exists(opts.spec_file):
                self.errorExit('--spec-file supplied but path does not exist')
            if os.path.isfile(opts.spec_file):
                if opts.input_file_name:
                    self.errorExit('Cannot use -i with --spec-file being a file')
                self.options.input_file_name = os.path.basename(opts.spec_file)
        if opts.verbose and opts.quiet:
            self.errorExit('Do not be an oxymoron with --verbose and --quiet')
        if opts.error and opts.allow_warnings:
            self.errorExit(f'Cannot use --error and --allow-warnings together')

        # Setup absolute paths and output paths
        if opts.output_dir:
            opts.output_dir = os.path.abspath(opts.output_dir)
            opts.results_file = os.path.join(opts.output_dir, opts.results_file)
        else:
            opts.results_file = os.path.abspath(opts.results_file)

        if opts.failed_tests and not os.path.exists(opts.results_file):
            self.errorExit('--failed-tests could not detect a previous run')

        # Update any keys from the environment as necessary
        if not self.options.method:
            if 'METHOD' in os.environ:
                self.options.method = os.environ['METHOD']
            else:
                self.options.method = 'opt'

        if not self.options.valgrind_mode:
            self.options.valgrind_mode = ''

        # Set default
        if not self.options.input_file_name:
            self.options.input_file_name = 'tests'

        if self.app_name is None:
            print('INFO: Setting --no-capabilities because there is not an application')
            self.options.no_capabilities = True


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
    # Bitterroot and windriver share software
    br_wr_config = HPCCluster(scheduler='slurm', apptainer_modules=inl_modules)
    hpc_configs = {'sawtooth': HPCCluster(scheduler='pbs',
                                          apptainer_modules=inl_modules),
                   'bitterroot': br_wr_config,
                   'windriver': br_wr_config}

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

    @staticmethod
    def buildRequiredCapabilities(registered: list[str],
                                  required: list[str]) -> list[Tuple[str, bool]]:
        """
        Helper for setting up the required capabilities.
        """
        assert isinstance(registered, list)
        assert isinstance(required, list)

        result = []
        for v in required:
            assert isinstance(v, str)
            v = v.strip()
            is_false = v[0] == '!'
            capability = v[1:] if is_false else v
            if capability not in registered:
                TestHarness.errorExit(f'Required capability "{capability}" is not registered')
            result.append((capability, is_false))
        return result

    @staticmethod
    def errorExit(*args):
        """
        Helper for printing an error and exiting
        """
        message = ' '.join([f'{v}' for v in args])
        raise SystemExit(f'ERROR: {message}')
