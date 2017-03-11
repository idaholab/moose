import re, os
import util
from InputParameters import InputParameters
from MooseObject import MooseObject

class Tester(MooseObject):

    @staticmethod
    def validParams():
        params = MooseObject.validParams()

        # Common Options
        params.addRequiredParam('type', "The type of test of Tester to create for this test.")
        params.addParam('max_time',   300, "The maximum in seconds that the test will be allowed to run.")
        params.addParam('min_reported_time', "The minimum time elapsed before a test is reported as taking to long to run.")
        params.addParam('skip',     "Provide a reason this test will be skipped.")
        params.addParam('deleted',         "Tests that only show up when using the '-e' option (Permanently skipped or not implemented).")

        params.addParam('heavy',    False, "Set to True if this test should only be run when the '--heavy' option is used.")
        params.addParam('group',       [], "A list of groups for which this test belongs.")
        params.addParam('prereq',      [], "A list of prereq tests that need to run successfully before launching this test.")
        params.addParam('skip_checks', False, "Tells the TestHarness to skip additional checks (This parameter is set automatically by the TestHarness during recovery tests)")
        params.addParam('scale_refine',    0, "The number of refinements to do when scaling")
        params.addParam('success_message', 'OK', "The successful message")

        params.addParam('cli_args',       [], "Additional arguments to be passed to the test.")

        params.addParam('valgrind', 'NONE', "Set to (NONE, NORMAL, HEAVY) to determine which configurations where valgrind will run.")

        # Test Filters
        params.addParam('platform',      ['ALL'], "A list of platforms for which this test will run on. ('ALL', 'DARWIN', 'LINUX', 'SL', 'LION', 'ML')")
        params.addParam('compiler',      ['ALL'], "A list of compilers for which this test is valid on. ('ALL', 'GCC', 'INTEL', 'CLANG')")
        params.addParam('petsc_version', ['ALL'], "A list of petsc versions for which this test will run on, supports normal comparison operators ('<', '>', etc...)")
        params.addParam('mesh_mode',     ['ALL'], "A list of mesh modes for which this test will run ('DISTRIBUTED', 'REPLICATED')")
        params.addParam('method',        ['ALL'], "A test that runs under certain executable configurations ('ALL', 'OPT', 'DBG', 'DEVEL', 'OPROF', 'PRO')")
        params.addParam('library_mode',  ['ALL'], "A test that only runs when libraries are built under certain configurations ('ALL', 'STATIC', 'DYNAMIC')")
        params.addParam('dtk',           ['ALL'], "A test that runs only if DTK is detected ('ALL', 'TRUE', 'FALSE')")
        params.addParam('unique_ids',    ['ALL'], "A test that runs only if UNIQUE_IDs are enabled ('ALL', 'TRUE', 'FALSE')")
        params.addParam('recover',       True,    "A test that runs with '--recover' mode enabled")
        params.addParam('vtk',           ['ALL'], "A test that runs only if VTK is detected ('ALL', 'TRUE', 'FALSE')")
        params.addParam('tecplot',       ['ALL'], "A test that runs only if Tecplot is detected ('ALL', 'TRUE', 'FALSE')")
        params.addParam('dof_id_bytes',  ['ALL'], "A test that runs only if libmesh is configured --with-dof-id-bytes = a specific number, e.g. '4', '8'")
        params.addParam('petsc_debug',   ['ALL'], "{False,True} -> test only runs when PETSc is configured with --with-debugging={0,1}, otherwise test always runs.")
        params.addParam('curl',          ['ALL'], "A test that runs only if CURL is detected ('ALL', 'TRUE', 'FALSE')")
        params.addParam('tbb',           ['ALL'], "A test that runs only if TBB is available ('ALL', 'TRUE', 'FALSE')")
        params.addParam('superlu',       ['ALL'], "A test that runs only if SuperLU is available via PETSc ('ALL', 'TRUE', 'FALSE')")
        params.addParam('slepc',         ['ALL'], "A test that runs only if SLEPc is available ('ALL', 'TRUE', 'FALSE')")
        params.addParam('unique_id',     ['ALL'], "A test that runs only if libmesh is configured with --enable-unique-id ('ALL', 'TRUE', 'FALSE')")
        params.addParam('cxx11',         ['ALL'], "A test that runs only if CXX11 is available ('ALL', 'TRUE', 'FALSE')")
        params.addParam('asio',          ['ALL'], "A test that runs only if ASIO is available ('ALL', 'TRUE', 'FALSE')")
        params.addParam('depend_files',  [], "A test that only runs if all depend files exist (files listed are expected to be relative to the base directory, not the test directory")
        params.addParam('env_vars',      [], "A test that only runs if all the environment variables listed exist")
        params.addParam('should_execute', True, 'Whether or not the executable needs to be run.  Use this to chain together multiple tests based off of one executeable invocation')
        params.addParam('required_submodule', [], "A list of initialized submodules for which this test requires.")
        params.addParam('check_input',    False, "Check for correct input file syntax")
        params.addParam('display_required', False, "The test requires and active display for rendering (i.e., ImageDiff tests).")

        return params

    def __init__(self, name, params):
        MooseObject.__init__(self, name, params)
        self.specs = params

        # Initialize the status bucket class
        self.status = util.TestStatus()

        # Enumerate the buckets here so ther are easier to work with in the tester class
        self.bucket_success = self.status.bucket_success
        self.bucket_fail    = self.status.bucket_fail
        self.bucket_diff    = self.status.bucket_diff
        self.bucket_pbs     = self.status.bucket_pbs
        self.bucket_pending = self.status.bucket_pending
        self.bucket_deleted = self.status.bucket_deleted
        self.bucket_skip    = self.status.bucket_skip
        self.bucket_silent  = self.status.bucket_silent

        # Initialize the tester with a pending status
        self.setStatus('launched', self.bucket_pending)

        # Set the status message
        if self.specs['check_input']:
            self.success_message = 'SYNTAX PASS'
        else:
            self.success_message = self.specs['success_message']

        # Set up common paramaters
        self.should_execute = self.specs['should_execute']
        self.check_input = self.specs['check_input']

    def getTestName(self):
        return self.specs['test_name']

    def getPrereqs(self):
        return self.specs['prereq']

    # Method to return if the test can run
    def getRunnable(self):
        return self.status.getRunnable()

    # Method to return text color based on current test status
    def getColor(self):
        return self.status.getColor()

    # Method to return the input file if applicable to this Tester
    def getInputFile(self):
        return None

    # Method to return the output files if applicable to this Tester
    def getOutputFiles(self):
        return []

    # Method to return the successful message printed to stdout
    def getSuccessMessage(self):
        return self.success_message

    # Method to return status text (exodiff, crash, skipped because x, y and z etc)
    def getStatusMessage(self):
        return self.status.getStatusMessage()

    # Method to return status bucket tuple
    def getStatus(self):
        return self.status.getStatus()

    # Method to set the bucket status
    def setStatus(self, reason, bucket):
        self.status.setStatus(reason, bucket)
        return self.getStatus()

    # Method to check if a test has failed. This method will return true if a
    # tester has failed at any point during the processing of the test.
    # Note: It's possible for a tester to report false for both didFail and
    #       didPass. This will happen if the tester is in-progress for instance.
    # See didPass()
    def didFail(self):
        return self.status.didFail()

    # Method to check for successfull test
    # Note: This method can return False until the tester has completely finished.
    #       For this reason it should be used only after the tester has completed.
    #       Instead you may want to use the didFail method which returns false
    #       only if the tester has failed at any point during the processing
    #       of that tester (e.g. after the main command has been run but before
    #       output has been tested).
    # See didFail()
    def didPass(self):
        return self.status.didPass()

    def getCheckInput(self):
        return self.check_input

    def setValgrindMode(self, mode):
        # Increase the alloted time for tests when running with the valgrind option
        if mode == 'NORMAL':
            self.specs['max_time'] = self.specs['max_time'] * 2
        elif mode == 'HEAVY':
            self.specs['max_time'] = self.specs['max_time'] * 6


    # Override this method to tell the harness whether or not this test should run.
    # This function should return a tuple (Boolean, reason)
    # If a reason is provided it'll be printed and counted as skipped.  If the reason
    # is left blank, the test will not be printed at all nor counted in the test totals.
    def checkRunnable(self, options):
        return (True, '')


    # Whether or not the executeable should be run
    # Don't override this
    def shouldExecute(self):
        return self.should_execute

    # This method is called prior to running the test.  It can be used to cleanup files
    # or do other preparations before the tester is run
    def prepare(self, options):
        return

    def getThreads(self, options):
        return 1

    def getProcs(self, options):
        return 1

    # This method should return the executable command that will be executed by the tester
    def getCommand(self, options):
        return

    # This method is called to return the commands (list) used for processing results
    def processResultsCommand(self, moose_dir, options):
        return []

    # This method will be called to process the results of running the test.  Any post-test
    # processing should happen in this method
    def processResults(self, moose_dir, retcode, options, output):
        return

    # This is the base level runnable check common to all Testers.  DO NOT override
    # this method in any of your derived classes.  Instead see "checkRunnable"
    def checkRunnableBase(self, options, checks, test_list=None):
        reason = ''

        # If --dry-run set the test status to pass and DO NOT return.
        # This will allow additional checks to perform and report tests
        # that would normally be skipped (and return as False).
        if options.dry_run:
            self.success_message = 'DRY RUN'
            self.setStatus(self.success_message, self.bucket_success)

        # Check if we only want to run failed tests
        if options.failed_tests:
            if self.specs['test_name'] not in test_list:
                self.setStatus('not failed', self.bucket_silent)
                return False

        # Are we running only tests in a specific group?
        if options.group <> 'ALL' and options.group not in self.specs['group']:
            self.setStatus('unmatched group', self.bucket_silent)
            return False
        if options.not_group <> '' and options.not_group in self.specs['group']:
            self.setStatus('unmatched group', self.bucket_silent)
            return False

        # Store regexp for matching tests if --re is used
        if options.reg_exp:
            match_regexp = re.compile(options.reg_exp)

        # If --re then only test matching regexp. Needs to run before other SKIP methods
        # This also needs to be in its own bucket group. We normally print skipped messages.
        # But we do not want to print tests that didn't match regex.
        if options.reg_exp and not match_regexp.search(self.specs['test_name']):
            self.setStatus('silent', self.bucket_silent)
            return False

        # Check for deleted tests
        if self.specs.isValid('deleted'):
            if options.extra_info:
                # We might want to trim the string so it formats nicely
                if len(self.specs['deleted']) >= util.TERM_COLS - (len(self.specs['test_name'])+21):
                    test_reason = (self.specs['deleted'])[:(util.TERM_COLS - (len(self.specs['test_name'])+24))] + '...'
                else:
                    test_reason = self.specs['deleted']
                reason = 'deleted (' + test_reason + ')'
            self.setStatus(reason, self.bucket_deleted)
            return False

        # Check for skipped tests
        if self.specs.type('skip') is bool and self.specs['skip']:
            # Backwards compatible (no reason)
            self.setStatus('no reason', self.bucket_skip)
            return False
        elif self.specs.type('skip') is not bool and self.specs.isValid('skip'):
            skip_message = self.specs['skip']
            # We might want to trim the string so it formats nicely
            if len(skip_message) >= util.TERM_COLS - (len(self.specs['test_name'])+21):
                reason = (skip_message)[:(util.TERM_COLS - (len(self.specs['test_name'])+24))] + '...'
            else:
                reason = skip_message
            self.setStatus(reason, self.bucket_skip)
            return False
        # If were testing for SCALE_REFINE, then only run tests with a SCALE_REFINE set
        elif (options.store_time or options.scaling) and self.specs['scale_refine'] == 0:
            self.setStatus('silent', self.bucket_silent)
            return False
        # If we're testing with valgrind, then skip tests that require parallel or threads or don't meet the valgrind setting
        elif options.valgrind_mode != '':
            if self.specs['valgrind'].upper() == 'NONE':
                reason = 'Valgrind==NONE'
            elif self.specs['valgrind'].upper() == 'HEAVY' and options.valgrind_mode.upper() == 'NORMAL':
                reason = 'Valgrind==HEAVY'
            elif self.specs['min_parallel'] > 1 or self.specs['min_threads'] > 1:
                reason = 'Valgrind requires serial'
            if reason != '':
                self.setStatus(reason, self.bucket_skip)
                return False
        # If we're running in recover mode skip tests that have recover = false
        elif options.enable_recover and self.specs['recover'] == False:
            reason = 'NO RECOVER'
            self.setStatus(reason, self.bucket_skip)
            return False

        # Check for PETSc versions
        (petsc_status, logic_reason, petsc_version) = util.checkPetscVersion(checks, self.specs)
        if not petsc_status:
            reason = 'using PETSc ' + str(checks['petsc_version']) + ' REQ: ' + logic_reason + ' ' + petsc_version
            self.setStatus(reason, self.bucket_skip)
            return False

        # PETSc is being explicitly checked above
        local_checks = ['platform', 'compiler', 'mesh_mode', 'method', 'library_mode', 'dtk', 'unique_ids', 'vtk', 'tecplot', \
                        'petsc_debug', 'curl', 'tbb', 'superlu', 'cxx11', 'asio', 'unique_id', 'slepc']
        for check in local_checks:
            test_platforms = set()
            operator_display = '!='
            inverse_set = False
            for x in self.specs[check]:
                if x[0] == '!':
                    if inverse_set:
                        reason = 'Multiple Negation Unsupported'
                        self.setStatus(reason, self.bucket_skip)
                        return False
                    inverse_set = True
                    operator_display = '=='
                    x = x[1:] # Strip off the !
                x_upper = x.upper()
                if x_upper in test_platforms:
                    reason = 'Duplicate Entry or Negative of Existing Entry'
                    self.setStatus(reason, self.bucket_skip)
                    return False
                test_platforms.add(x.upper())

            match_found = len(test_platforms.intersection(checks[check])) > 0
            # Either we didn't find the match when we were using normal "include" logic
            # or we did find the match when we wanted to exclude it
            if inverse_set == match_found:
                reason = re.sub(r'\[|\]', '', check).upper() + operator_display + ', '.join(test_platforms)
                self.setStatus(reason, self.bucket_skip)
                return False

        # Check for heavy tests
        if options.all_tests or options.heavy_tests:
            if not self.specs['heavy'] and options.heavy_tests:
                reason = 'NOT HEAVY'
                self.setStatus(reason, self.bucket_silent)
                return False
        elif self.specs['heavy']:
            reason = 'HEAVY'
            self.setStatus(reason, self.bucket_skip)
            return False

        # Check for positive scale refine values when using store timing options
        if self.specs['scale_refine'] == 0 and options.store_time:
            self.setStatus('scale_refine==0 store_time=True', self.bucket_skip)
            return False

        # There should only be one entry in self.specs['dof_id_bytes']
        for x in self.specs['dof_id_bytes']:
            if x != 'ALL' and not x in checks['dof_id_bytes']:
                reason = '--with-dof-id-bytes!=' + x
                self.setStatus(reason, self.bucket_skip)
                return False

        # Check to make sure depend files exist
        for file in self.specs['depend_files']:
            if not os.path.isfile(os.path.join(self.specs['base_dir'], file)):
                reason = 'DEPEND FILES'
                self.setStatus(reason, self.bucket_skip)
                return False

        # Check to make sure required submodules are initialized
        for var in self.specs['required_submodule']:
            if var not in checks["submodules"]:
                reason = '%s submodule not initialized' % var
                self.setStatus(reason, self.bucket_skip)
                return False

        # Check to make sure environment variable exists
        for var in self.specs['env_vars']:
            if not os.environ.has_key(var):
                reason = 'ENV VAR NOT SET'
                self.setStatus(reason, self.bucket_skip)
                return False

        # Check for display
        if self.specs['display_required'] and not os.getenv('DISPLAY', False):
            reason = 'NO DISPLAY'
            self.setStatus(reason, self.bucket_skip)
            return False

        # Check the return values of the derived classes
        return self.checkRunnable(options)
