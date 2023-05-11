#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import platform, re, os, sys, pkgutil, shutil
import mooseutils
from TestHarness import util
from TestHarness.StatusSystem import StatusSystem
from FactorySystem.MooseObject import MooseObject
from tempfile import SpooledTemporaryFile
import subprocess
from signal import SIGTERM

class Tester(MooseObject):
    """
    Base class from which all tester objects are instanced.
    """
    @staticmethod
    def validParams():
        params = MooseObject.validParams()

        # Common Options
        params.addRequiredParam('type', "The type of test of Tester to create for this test.")
        params.addParam('max_time',   int(os.getenv('MOOSE_TEST_MAX_TIME', 300)), "The maximum in seconds that the test will be allowed to run.")
        params.addParam('skip',     "Provide a reason this test will be skipped.")
        params.addParam('deleted',         "Tests that only show up when using the '-e' option (Permanently skipped or not implemented).")
        params.addParam('unique_test_id', "The unique hash given to a test")

        params.addParam('heavy',    False, "Set to True if this test should only be run when the '--heavy' option is used.")
        params.addParam('group',       [], "A list of groups for which this test belongs.")
        params.addParam('prereq',      [], "A list of prereq tests that need to run successfully before launching this test. When 'prereq = ALL', TestHarness will run this test last. Multiple 'prereq = ALL' tests, or tests that depend on a 'prereq = ALL' test will result in cyclic errors. Naming a test 'ALL' when using 'prereq = ALL' will also result in an error.")
        params.addParam('skip_checks', False, "Tells the TestHarness to skip additional checks (This parameter is set automatically by the TestHarness during recovery tests)")
        params.addParam('scale_refine',    0, "The number of refinements to do when scaling")
        params.addParam('success_message', 'OK', "The successful message")

        params.addParam('cli_args',       [], "Additional arguments to be passed to the test.")
        params.addParam('allow_test_objects', False, "Allow the use of test objects by adding --allow-test-objects to the command line.")

        params.addParam('valgrind', 'NONE', "Set to (NONE, NORMAL, HEAVY) to determine which configurations where valgrind will run.")
        params.addParam('tags',      [], "A list of strings")
        params.addParam('max_buffer_size', None, "Bytes allowed in stdout/stderr before it is subjected to being trimmed. Set to -1 to ignore output size restrictions. "
                                                 "If 'max_buffer_size' is not set, the default value of 'None' triggers a reasonable value (e.g. 100 kB)")
        params.addParam('parallel_scheduling', False, "Allow all tests in test spec file to run in parallel (adheres to prereq rules).")

        # Test Filters
        params.addParam('platform',      ['ALL'], "A list of platforms for which this test will run on. ('ALL', 'DARWIN', 'LINUX', 'SL', 'LION', 'ML')")
        params.addParam('compiler',      ['ALL'], "A list of compilers for which this test is valid on. ('ALL', 'GCC', 'INTEL', 'CLANG')")
        params.addParam('petsc_version', ['ALL'], "A list of petsc versions for which this test will run on, supports normal comparison operators ('<', '>', etc...)")
        params.addParam('petsc_version_release', ['ALL'], "A test that runs against PETSc master if FALSE ('ALL', 'TRUE', 'FALSE')")
        params.addParam('slepc_version', [], "A list of slepc versions for which this test will run on, supports normal comparison operators ('<', '>', etc...)")
        params.addParam('exodus_version', ['ALL'], "A list of Exodus versions for which this test will run on, supports normal comparison operators ('<', '>', etc...)")
        params.addParam('vtk_version', ['ALL'], "A list of VTK versions for which this test will run on, supports normal comparison operators ('<', '>', etc...)")
        params.addParam('mesh_mode',     ['ALL'], "A list of mesh modes for which this test will run ('DISTRIBUTED', 'REPLICATED')")
        params.addParam('min_ad_size',   None, "A minimum AD size for which this test will run")
        params.addParam('method',        ['ALL'], "A test that runs under certain executable configurations ('ALL', 'OPT', 'DBG', 'DEVEL', 'OPROF', 'PRO')")
        params.addParam('library_mode',  ['ALL'], "A test that only runs when libraries are built under certain configurations ('ALL', 'STATIC', 'DYNAMIC')")
        params.addParam('dtk',           ['ALL'], "A test that runs only if DTK is detected ('ALL', 'TRUE', 'FALSE')")
        params.addParam('unique_ids',    ['ALL'], "Deprecated. Use unique_id instead.")
        params.addParam('recover',       True,    "A test that runs with '--recover' mode enabled")
        params.addParam('vtk',           ['ALL'], "A test that runs only if VTK is detected ('ALL', 'TRUE', 'FALSE')")
        params.addParam('tecplot',       ['ALL'], "A test that runs only if Tecplot is detected ('ALL', 'TRUE', 'FALSE')")
        params.addParam('dof_id_bytes',  ['ALL'], "A test that runs only if libmesh is configured --with-dof-id-bytes = a specific number, e.g. '4', '8'")
        params.addParam('petsc_debug',   ['ALL'], "{False,True} -> test only runs when PETSc is configured with --with-debugging={0,1}, otherwise test always runs.")
        params.addParam('curl',          ['ALL'], "A test that runs only if CURL is detected ('ALL', 'TRUE', 'FALSE')")
        params.addParam('threading',     ['ALL'], "A list of threading models ths tests runs with ('ALL', 'TBB', 'OPENMP', 'PTHREADS', 'NONE')")
        params.addParam('superlu',       ['ALL'], "A test that runs only if SuperLU is available via PETSc ('ALL', 'TRUE', 'FALSE')")
        params.addParam('mumps',         ['ALL'], "A test that runs only if MUMPS is available via PETSc ('ALL', 'TRUE', 'FALSE')")
        params.addParam('strumpack',     ['ALL'], "A test that runs only if STRUMPACK is available via PETSc ('ALL', 'TRUE', 'FALSE')")
        params.addParam('chaco',         ['ALL'], "A test that runs only if Chaco (partitioner) is available via PETSc ('ALL', 'TRUE', 'FALSE')")
        params.addParam('parmetis',      ['ALL'], "A test that runs only if Parmetis (partitioner) is available via PETSc ('ALL', 'TRUE', 'FALSE')")
        params.addParam('party',         ['ALL'], "A test that runs only if Party (partitioner) is available via PETSc ('ALL', 'TRUE', 'FALSE')")
        params.addParam('ptscotch',      ['ALL'], "A test that runs only if PTScotch (partitioner) is available via PETSc ('ALL', 'TRUE', 'FALSE')")
        params.addParam('slepc',         ['ALL'], "A test that runs only if SLEPc is available ('ALL', 'TRUE', 'FALSE')")
        params.addParam('unique_id',     ['ALL'], "A test that runs only if libmesh is configured with --enable-unique-id ('ALL', 'TRUE', 'FALSE')")
        params.addParam('cxx11',         ['ALL'], "A test that runs only if CXX11 is available ('ALL', 'TRUE', 'FALSE')")
        params.addParam('asio',          ['ALL'], "A test that runs only if ASIO is available ('ALL', 'TRUE', 'FALSE')")
        params.addParam('fparser_jit',   ['ALL'], "A test that runs only if FParser JIT is available ('ALL', 'TRUE', 'FALSE')")
        params.addParam('libpng',        ['ALL'], "A test that runs only if libpng is available ('ALL', 'TRUE', 'FALSE')")
        params.addParam('libtorch',      ['ALL'], "A test that runs only if libtorch is available ('ALL', 'TRUE', 'FALSE')")
        params.addParam('libtorch_version', ['ALL'], "A list of libtorch versions for which this test will run on, supports normal comparison operators ('<', '>', etc...)")
        params.addParam('installation_type',['ALL'], "A test that runs under certain executable installation configurations ('ALL', 'IN_TREE', 'RELOCATED')")

        params.addParam('depend_files',  [], "A test that only runs if all depend files exist (files listed are expected to be relative to the base directory, not the test directory")
        params.addParam('env_vars',      [], "A test that only runs if all the environment variables listed exist")
        params.addParam('should_execute', True, 'Whether or not the executable needs to be run.  Use this to chain together multiple tests based off of one executeable invocation')
        params.addParam('required_submodule', [], "A list of initialized submodules for which this test requires.")
        params.addParam('required_objects', [], "A list of required objects that are in the executable.")
        params.addParam('required_applications', [], "A list of required registered applications that are in the executable.")
        params.addParam('check_input',    False, "Check for correct input file syntax")
        params.addParam('display_required', False, "The test requires and active display for rendering (i.e., ImageDiff tests).")
        params.addParam('timing',         True, "If True, the test will be allowed to run with the timing flag (i.e. Manually turning on performance logging).")
        params.addParam('boost',         ['ALL'], "A test that runs only if BOOST is detected ('ALL', 'TRUE', 'FALSE')")
        params.addParam('python',        None, "Restrict the test to s specific version of python (e.g., 3.6 or 3.7.1).")
        params.addParam('required_python_packages', None, "Test will only run if the supplied python packages exist.")
        params.addParam('requires', None, "A list of programs required for the test to operate, as tested with shutil.which.")
        params.addParam("working_directory", None, "When set, TestHarness will enter this directory before running test")

        # SQA
        params.addParam("requirement", None, "The SQA requirement that this test satisfies (e.g., 'The Marker system shall provide means to mark elements for refinement within a box region.')")
        params.addParam("design", [], "The list of markdown files that contain the design(s) associated with this test (e.g., '/Markers/index.md /BoxMarker.md').")
        params.addParam("issues", [], "The list of github issues associated with this test (e.g., '#1234 #4321')")
        params.addParam("detail", None, "Details of SQA requirement for use within sub-blocks.")
        params.addParam("validation", False, "Set to True to mark test as a validation problem.")
        params.addParam("verification", False, "Set to True to mark test as a verification problem.")
        params.addParam("deprecated", False, "When True the test is no longer considered part SQA process and as such does not include the need for a requirement definition.")
        params.addParam("collections", [], "A means for defining a collection of tests for SQA process.")
        params.addParam("classification", 'functional', "A means for defining a requirement classification for SQA process.")
        return params

    # This is what will be checked for when we look for valid testers
    IS_TESTER = True

    def __init__(self, name, params):
        MooseObject.__init__(self, name, params)
        self.specs = params
        self.outfile = None
        self.errfile = None
        self.joined_out = ''
        self.exit_code = 0
        self.process = None
        self.tags = params['tags']
        self.__caveats = set([])

        # Alternate text we want to print as part of our status instead of the
        # pre-formatted status text (SYNTAX PASS instead of OK for example)
        self.__tester_message = ''

        # Bool if test can run
        self._runnable = None

        # Set up common paramaters
        self.should_execute = self.specs['should_execute']
        self.check_input = self.specs['check_input']

        if self.specs["allow_test_objects"]:
            self.specs["cli_args"].append("--allow-test-objects")

        ### Enumerate the tester statuses we want to use
        self.test_status = StatusSystem()
        self.no_status = self.test_status.no_status
        self.queued = self.test_status.queued
        self.skip = self.test_status.skip
        self.silent = self.test_status.silent
        self.success = self.test_status.success
        self.fail = self.test_status.fail
        self.diff = self.test_status.diff
        self.deleted = self.test_status.deleted

        self.__failed_statuses = self.test_status.getFailingStatuses()
        self.__skipped_statuses = [self.skip, self.silent]

    def getStatus(self):
        return self.test_status.getStatus()

    def setStatus(self, status, message=''):
        self.__tester_message = message
        return self.test_status.setStatus(status)

    def createStatus(self):
        return self.test_status.createStatus()

    # Return a tuple (status, message, caveats) for this tester as found
    # in the .previous_test_results.json file (or supplied json object)
    def previousTesterStatus(self, options, previous_storage=None):
        if not previous_storage:
            previous_storage = options.results_storage

        status_exists = previous_storage.get(self.getTestDir(), {}).get(self.getTestName(), None)
        status = (self.test_status.createStatus(), '', '')
        if status_exists:
            status = (self.test_status.createStatus(str(status_exists['STATUS'])),
                      str(status_exists['STATUS_MESSAGE']),
                      status_exists['CAVEATS'])
        return (status)

    def getStatusMessage(self):
        return self.__tester_message

    # Return a boolean based on current status
    def isNoStatus(self):
        return self.getStatus() == self.no_status
    def isSkip(self):
        return self.getStatus() in self.__skipped_statuses
    def isQueued(self):
        return self.getStatus() == self.queued
    def isSilent(self):
        return self.getStatus() == self.silent
    def isPass(self):
        return self.getStatus() == self.success
    def isFail(self):
        return self.getStatus() in self.__failed_statuses
    def isDiff(self):
        return self.getStatus() == self.diff
    def isDeleted(self):
        return self.getStatus() == self.deleted

    def getTestName(self):
        """ return test name """
        return self.specs['test_name']

    def getPrereqs(self):
        """ return list of prerequisite tests this test depends on """
        return self.specs['prereq']

    def getMooseDir(self):
        """ return moose directory """
        return self.specs['moose_dir']

    def getMoosePythonDir(self):
        """ return moose directory """
        return self.specs['moose_python_dir']

    def getTestDir(self):
        """ return directory in which this tester is located """
        if self.specs['working_directory']:
            return os.path.join(self.specs['test_dir'], self.specs['working_directory'])
        return self.specs['test_dir']

    def getMinReportTime(self):
        """ return minimum time elapse before reporting a 'long running' status """
        return self.specs['min_reported_time']

    def getMaxTime(self):
        """ return maximum time elapse before reporting a 'timeout' status """
        return float(self.specs['max_time'])

    def getUniqueTestID(self):
        """ return unique hash for test """
        return self.specs['unique_test_id']

    def getRunnable(self, options):
        """ return bool and cache results, if this test can run """
        if self._runnable is None:
            self._runnable = self.checkRunnableBase(options)
        return self._runnable

    def getInputFile(self):
        """ return the input file if applicable to this Tester """
        return None

    def getInputFileContents(self):
        """ return the contents of the input file applicable to this Tester """
        return None

    def getOutputFiles(self):
        """ return the output files if applicable to this Tester """
        return []

    def getOutput(self):
        """ Return the contents of stdout and stderr """
        return self.joined_out

    def getCheckInput(self):
        return self.check_input

    def setValgrindMode(self, mode):
        """ Increase the alloted time for tests when running with the valgrind option """
        if mode == 'NORMAL':
            self.specs['max_time'] = float(self.specs['max_time']) * 2
        elif mode == 'HEAVY':
            self.specs['max_time'] = float(self.specs['max_time']) * 6

    def checkRunnable(self, options):
        """
        Derived method to return tuple if this tester should be executed or not.

        The tuple should be structured as (boolean, 'reason'). If false, and the
        reason is left blank, this tester will be treated as silent (no status
        will be printed and will not be counted among the skipped tests).
        """
        return True

    def shouldExecute(self):
        """
        return boolean for tester allowed to execute its command
        see .getCommand for more information
        """
        return self.should_execute

    def prepare(self, options):
        """
        Method which is called prior to running the test. It can be used to cleanup files
        or do other preparations before the tester is run.
        """
        return

    def getThreads(self, options):
        """ return number of threads to use for this tester """
        return 1

    def getProcs(self, options):
        """ return number of processors to use for this tester """
        return 1

    def getSlots(self, options):
        """ return number of slots to use for this tester """
        return self.getThreads(options) * self.getProcs(options)

    def getCommand(self, options):
        """ return the executable command that will be executed by the tester """
        return ''

    def runCommand(self, cmd, cwd, timer, options):
        """
        Helper method for running external (sub)processes as part of the tester's execution.  This
        uses the tester's getCommand and getTestDir methods to run a subprocess.  The timer must
        be the same timer passed to the run method.  Results from running the subprocess is stored
        in the tester's output and exit_code fields.
        """

        cmd = self.getCommand(options)
        cwd = self.getTestDir()

        # Verify that the working directory is available right before we execute.
        if not os.path.exists(cwd):
            # Timers must be used since they are directly indexed in the Job class
            timer.start()
            self.setStatus(self.fail, 'WORKING DIRECTORY NOT FOUND')
            timer.stop()
            return

        self.process = None
        try:
            f = SpooledTemporaryFile(max_size=1000000) # 1M character buffer
            e = SpooledTemporaryFile(max_size=100000)  # 100K character buffer

            # On Windows, there is an issue with path translation when the command is passed in
            # as a list.
            if platform.system() == "Windows":
                process = subprocess.Popen(cmd, stdout=f, stderr=e, close_fds=False,
                                           shell=True, creationflags=subprocess.CREATE_NEW_PROCESS_GROUP, cwd=cwd)
            else:
                process = subprocess.Popen(cmd, stdout=f, stderr=e, close_fds=False,
                                           shell=True, preexec_fn=os.setsid, cwd=cwd)
        except:
            print("Error in launching a new task", cmd)
            raise

        self.process = process
        self.outfile = f
        self.errfile = e

        timer.start()
        process.wait()
        timer.stop()

        self.exit_code = process.poll()
        self.outfile.flush()
        self.errfile.flush()

        # store the contents of output, and close the file
        self.joined_out = util.readOutput(self.outfile, self.errfile, self)
        self.outfile.close()
        self.errfile.close()

    def killCommand(self):
        """
        Kills any currently executing process started by the runCommand method.
        """
        if self.process is not None:
            try:
                if platform.system() == "Windows":
                    from distutils import spawn
                    if spawn.find_executable("taskkill"):
                        subprocess.call(['taskkill', '/F', '/T', '/PID', str(self.process.pid)])
                    else:
                        self.process.terminate()
                else:
                    pgid = os.getpgid(self.process.pid)
                    os.killpg(pgid, SIGTERM)
            except OSError: # Process already terminated
                pass

    def run(self, timer, options):
        """
        This is a method that is the tester's main execution code.  Subclasses can override this
        method with custom code relevant to their specific testing needs.  By default this method
        calls runCommand.  runCommand is provided as a helper for running (external) subprocesses
        as part of the tester's execution and should be the *only* way subprocesses are executed
        if needed. The run method is responsible to call the start+stop methods on timer to record
        the time taken to run the actual test.  start+stop can be called multiple times.
        """
        cmd = self.getCommand(options)
        cwd = self.getTestDir()

        self.runCommand(cmd, cwd, timer, options)

    def processResultsCommand(self, moose_dir, options):
        """ method to return the commands (list) used for processing results """
        return []

    def processResults(self, moose_dir, options, output):
        """ method to process the results of a finished tester """
        return

    def hasRedirectedOutput(self, options):
        """ return bool on tester having redirected output """
        return (self.specs.isValid('redirect_output') and self.specs['redirect_output'] == True and self.getProcs(options) > 1)

    def getRedirectedOutputFiles(self, options):
        """ return a list of redirected output """
        return [os.path.join(self.getTestDir(), self.name() + '.processor.{}'.format(p)) for p in range(self.getProcs(options))]

    def addCaveats(self, *kwargs):
        """ Add caveat(s) which will be displayed with the final test status """
        for i in [x for x in kwargs if x]:
            if type(i) == type([]):
                self.__caveats.update(i)
            else:
                self.__caveats.add(i)
        return self.getCaveats()

    def getCaveats(self):
        """ Return caveats accumalted by this tester """
        return self.__caveats

    def clearCaveats(self):
        """ Clear any caveats stored in tester """
        self.__caveats = set([])
        return self.getCaveats()

    def checkRunnableBase(self, options):
        """
        Method to check for caveats that would prevent this tester from
        executing correctly (or not at all).

        DO NOT override this method. Instead, see .checkRunnable()
        """
        reasons = {}
        checks = options._checks

        tag_match = False
        for t in self.tags:
            if t in options.runtags:
                tag_match = True
                break
        if len(options.runtags) > 0 and not tag_match:
            self.setStatus(self.silent)
            return False

        # If something has already deemed this test a failure
        if self.isFail():
            return False

        # Check if we only want to run syntax tests
        if options.check_input and not self.specs['check_input']:
            self.setStatus(self.silent)
            return False

        # Check if we want to exclude syntax tests
        if options.no_check_input and self.specs['check_input']:
            self.setStatus(self.silent)
            return False

        # Are we running only tests in a specific group?
        if options.group != 'ALL' and options.group not in self.specs['group']:
            self.setStatus(self.silent)
            return False
        if options.not_group != '' and options.not_group in self.specs['group']:
            self.setStatus(self.silent)
            return False

        # Store regexp for matching tests if --re is used
        if options.reg_exp:
            match_regexp = re.compile(options.reg_exp)

        # If --re then only test matching regexp. Needs to run before other SKIP methods
        # This also needs to be in its own bucket group. We normally print skipped messages.
        # But we do not want to print tests that didn't match regex.
        if options.reg_exp and not match_regexp.search(self.specs['test_name']):
            self.setStatus(self.silent)
            return False

        # Short circuit method and run this test if we are ignoring all caveats
        if options.ignored_caveats == 'all':
            # Still, we should abide by the derived classes
            return self.checkRunnable(options)

        # Check for deleted tests
        if self.specs.isValid('deleted'):
            reasons['deleted'] = str(self.specs['deleted'])

        # Skipped by external means (example: TestHarness part2 with --check-input)
        if self.isSkip() and self.getStatusMessage():
            reasons['skip'] = self.getStatusMessage()
        # Test is skipped
        elif self.specs.type('skip') is bool and self.specs['skip']:
            # Backwards compatible (no reason)
            reasons['skip'] = 'no reason'
        elif self.specs.type('skip') is not bool and self.specs.isValid('skip'):
            reasons['skip'] = self.specs['skip']
        # If were testing for SCALE_REFINE, then only run tests with a SCALE_REFINE set
        elif (options.scaling) and self.specs['scale_refine'] == 0:
            self.setStatus(self.silent)
            return False
        # If we're testing with valgrind, then skip tests that require parallel or threads or don't meet the valgrind setting
        elif options.valgrind_mode != '':
            tmp_reason = ''
            if self.specs['valgrind'].upper() != options.valgrind_mode.upper():
                tmp_reason = 'Valgrind==' + self.specs['valgrind'].upper()
            elif int(self.specs['min_threads']) > 1:
                tmp_reason = 'Valgrind requires non-threaded'
            elif self.specs["check_input"]:
                tmp_reason = 'check_input==True'
            if tmp_reason != '':
                reasons['valgrind'] = tmp_reason
        # If we're running in recover mode skip tests that have recover = false
        elif options.enable_recover and self.specs['recover'] == False:
            reasons['recover'] = 'NO RECOVER'

        # AD size check
        ad_size = int(util.getMooseConfigOption(self.specs['moose_dir'], 'ad_size').pop())
        min_ad_size = self.specs['min_ad_size']
        if min_ad_size is not None and int(min_ad_size) > ad_size:
            reasons['min_ad_size'] = "Minimum AD size %d needed, but MOOSE is configured with %d" % (int(min_ad_size), ad_size)

        # Check for PETSc versions
        (petsc_status, petsc_version) = util.checkPetscVersion(checks, self.specs)
        if not petsc_status:
            reasons['petsc_version'] = 'using PETSc ' + str(checks['petsc_version']) + ' REQ: ' + petsc_version

        # Check for SLEPc versions
        (slepc_status, slepc_version) = util.checkSlepcVersion(checks, self.specs)
        if not slepc_status and len(self.specs['slepc_version']) != 0:
            if slepc_version != None:
                reasons['slepc_version'] = 'using SLEPc ' + str(checks['slepc_version']) + ' REQ: ' + slepc_version
            elif slepc_version == None:
                reasons['slepc_version'] = 'SLEPc is not installed'

        # Check for ExodusII versions
        (exodus_status, exodus_version) = util.checkExodusVersion(checks, self.specs)
        if not exodus_status:
            if checks['exodus_version'] != None:
                reasons['exodus_version'] = 'using ExodusII ' + str(checks['exodus_version']) + ' REQ: ' + exodus_version
            else:
                reasons['exodus_version'] = 'Exodus version not detected. REQ: ' + exodus_version

        # Check for VTK versions
        (vtk_status, vtk_version) = util.checkVTKVersion(checks, self.specs)
        if not vtk_status:
            if checks['vtk_version'] != None:
                reasons['vtk_version'] = 'using VTK ' + str(checks['vtk_version']) + ' REQ: ' + vtk_version
            else:
                reasons['vtk_version'] = 'VTK version not detected. REQ: ' + vtk_version

        # Check for libtorch version
        (libtorch_status, libtorch_version) = util.checkLibtorchVersion(checks, self.specs)
        if not libtorch_status:
            reasons['libtorch_version'] = 'using libtorch ' + str(checks['libtorch_version']) + ' REQ: ' + libtorch_version

        # PETSc and SLEPc is being explicitly checked above
        local_checks = ['platform', 'compiler', 'mesh_mode', 'method', 'library_mode', 'dtk',
                        'unique_ids', 'vtk', 'tecplot', 'petsc_debug', 'curl', 'superlu', 'mumps',
                        'strumpack', 'cxx11', 'asio', 'unique_id', 'slepc', 'petsc_version_release',
                        'boost', 'fparser_jit', 'parmetis', 'chaco', 'party', 'ptscotch',
                        'threading', 'libpng', 'libtorch']

        for check in local_checks:
            test_platforms = set()
            operator_display = '!='
            inverse_set = False
            for x in self.specs[check]:
                if x[0] == '!':
                    if inverse_set:
                        reasons[check] = 'Multiple Negation Unsupported'
                    inverse_set = True
                    operator_display = '=='
                    x = x[1:] # Strip off the !
                x_upper = x.upper()
                if x_upper in test_platforms:
                    reasons[x_upper] = 'Duplicate Entry or Negative of Existing Entry'
                test_platforms.add(x.upper())

            match_found = len(test_platforms.intersection(checks[check])) > 0
            # Either we didn't find the match when we were using normal "include" logic
            # or we did find the match when we wanted to exclude it
            if inverse_set == match_found:
                reasons[check] = re.sub(r'\[|\]', '', check).upper() + operator_display + ', '.join(test_platforms)

        # Check for binary location
        if (self.specs['installation_type'] and
            self.specs['installation_type'][0].upper() not in checks['installation_type']):
            reasons['installation_type'] = f'test requires "{self.specs["installation_type"][0]}" binary'

        # Check for heavy tests
        if options.all_tests or options.heavy_tests:
            if not self.specs['heavy'] and options.heavy_tests:
                reasons['heavy'] = 'NOT HEAVY'
        elif self.specs['heavy']:
            reasons['heavy'] = 'HEAVY'

        # There should only be one entry in self.specs['dof_id_bytes']
        for x in self.specs['dof_id_bytes']:
            if x != 'ALL' and not x in checks['dof_id_bytes']:
                reasons['dof_id_bytes'] = '--with-dof-id-bytes!=' + x

        # Check to make sure depend files exist
        for file in self.specs['depend_files']:
            if not os.path.isfile(os.path.join(self.specs['base_dir'], file)):
                reasons['depend_files'] = 'DEPEND FILES'

        # We calculate the exe_objects only if we need them
        if self.specs["required_objects"] and checks["exe_objects"] is None:
            checks["exe_objects"] = util.getExeObjects(self.specs["executable"])

        # Check to see if we have the required object names
        for var in self.specs['required_objects']:
            if var not in checks["exe_objects"]:
                reasons['required_objects'] = '%s not found in executable' % var
                break

        # We extract the registered apps only if we need them
        if self.specs["required_applications"] and checks["registered_apps"] is None:
            checks["registered_apps"] = util.getRegisteredApps(self.specs["executable"],
                                                               self.specs["app_name"])

        # Check to see if we have the required application names
        for var in self.specs['required_applications']:
            if var.upper() not in checks["registered_apps"]:
                reasons['required_applications'] = 'App %s not registered in executable' % var
                break

        # Check to make sure required submodules are initialized
        for var in self.specs['required_submodule']:
            if var not in checks["submodules"]:
                reasons['required_submodule'] = '%s submodule not initialized' % var

        # Check to make sure environment variable exists
        for var in self.specs['env_vars']:
            if not os.environ.get(var):
                reasons['env_vars'] = 'ENV VAR NOT SET'

        # Check for display
        if self.specs['display_required'] and not os.getenv('DISPLAY', False):
            reasons['display_required'] = 'NO DISPLAY'

        # Check python version
        py_version = self.specs['python']
        if (py_version is not None):
            if isinstance(py_version, int) and (py_version != sys.version_info[0]):
                reasons['python'] = 'PYTHON != {}'.format(py_version)
            elif isinstance(py_version, float) and (py_version != float('{}.{}'.format(*sys.version_info[0:2]))):
                reasons['python'] = 'PYTHON != {}'.format(py_version)
            elif isinstance(py_version, str):
                ver = py_version.split('.')
                if any(sys.version_info[i] != int(v) for i, v in enumerate(ver)):
                    reasons['python'] = 'PYTHON != {}'.format(py_version)

        # Check python packages
        py_packages = self.specs['required_python_packages']
        if py_packages is not None:
            missing = mooseutils.check_configuration(py_packages.split(), message=False)
            if missing:
                reasons['python_packages_required'] = ', '.join(['no {}'.format(p) for p in missing])

        # Check for programs
        programs = self.specs['requires']
        if (programs is not None):
            missing = []
            for prog in programs.split():
                if shutil.which(prog) is None:
                    missing.append(prog)
            if missing:
                reasons['requires'] = ', '.join(['no {}'.format(p) for p in missing])

        # Verify working_directory is relative. We'll check to make sure it's available just in time.
        if self.specs['working_directory']:
            if self.specs['working_directory'][:1] == os.path.sep:
                self.setStatus(self.fail, 'ABSOLUTE PATH DETECTED')

        # We can't offer the option of reading output files outside of initial TestDir
        if self.specs['working_directory'] and (options.pbs
                                                or options.ok_files
                                                or options.fail_files
                                                or options.sep_files):
            reasons['working_directory'] = '--sep-files* enabled'

        ##### The below must be performed last to register all above caveats #####
        # Remove any matching user supplied caveats from accumulated checkRunnable caveats that
        # would normally produce a skipped test.
        caveat_list = set()
        if options.ignored_caveats:
            caveat_list = set([x.lower() for x in options.ignored_caveats.split()])

        if len(set(reasons.keys()) - caveat_list) > 0:
            tmp_reason = []
            for key, value in reasons.items():
                if key.lower() not in caveat_list:
                    tmp_reason.append(value)

            flat_reason = ', '.join(tmp_reason)

            # If the test is deleted we still need to treat this differently
            self.addCaveats(flat_reason)
            if 'deleted' in reasons.keys():
                if options.extra_info:
                    self.setStatus(self.deleted)
                else:
                    self.setStatus(self.silent)
            elif self.getStatus() == self.fail:
                return False
            else:
                self.setStatus(self.skip)
            return False

        # Check the return values of the derived classes
        self._runnable = self.checkRunnable(options)
        return self._runnable
