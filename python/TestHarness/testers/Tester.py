import re, os
import util
from InputParameters import InputParameters
from MooseObject import MooseObject
from timeit import default_timer as clock

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
        params.addParam('petsc_version_release', ['ALL'], "A test that runs against PETSc master if FALSE ('ALL', 'TRUE', 'FALSE')")
        params.addParam('slepc_version', [], "A list of slepc versions for which this test will run on, supports normal comparison operators ('<', '>', etc...)")
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
        params.addParam('required_objects', [], "A list of required objects that are in the executable.")
        params.addParam('check_input',    False, "Check for correct input file syntax")
        params.addParam('display_required', False, "The test requires and active display for rendering (i.e., ImageDiff tests).")
        params.addParam('boost',         ['ALL'], "A test that runs only if BOOT is detected ('ALL', 'TRUE', 'FALSE')")

        return params

    def __init__(self, name, params):
        MooseObject.__init__(self, name, params)
        self.specs = params

        # Bool if test can run
        self._runnable = None

        ### several variables needed when performing processResults
        self.__start_time = clock()
        self.__end_time = None
        self.__exit_code = 0
        self.__std_out = ''

        # Initialize the status bucket class
        self.status = util.TestStatus()

        # Enumerate the buckets here so ther are easier to work with in the tester class
        self.bucket_initialized  = self.status.bucket_initialized
        self.bucket_success      = self.status.bucket_success
        self.bucket_fail         = self.status.bucket_fail
        self.bucket_diff         = self.status.bucket_diff
        self.bucket_pending      = self.status.bucket_pending
        self.bucket_finished     = self.status.bucket_finished
        self.bucket_deleted      = self.status.bucket_deleted
        self.bucket_skip         = self.status.bucket_skip
        self.bucket_silent       = self.status.bucket_silent

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

    def getMooseDir(self):
        return self.specs['moose_dir']

    def getTestDir(self):
        return self.specs['test_dir']

    def getExitCode(self):
        return self.__exit_code

    def setExitCode(self, exit_code):
        self.__exit_code = exit_code
        return self.__exit_code

    def getOutput(self):
        return self.__std_out

    def setOutput(self, output):
        self.__std_out = output
        return self.__std_out

    def getStartTime(self):
        return self.__start_time

    def setStartTime(self, start_time):
        self.__start_time = start_time
        return self.__start_time

    def getEndTime(self):
        return self.__end_time

    def setEndTime(self, end_time):
        self.__end_time = end_time
        return self.__end_time

    def getPerfTime(self):
        return self.__perf_time

    def setPerfTime(self, perf_time):
        self.perf_time = perf_time
        return self.__perf_time

    def getMinReportTime(self):
        return self.specs['min_reported_time']

    def getMaxTime(self):
        return self.specs['max_time']

    # A cached method to return if the test can run
    def getRunnable(self, options):
        if self._runnable is None:
            self._runnable = self.checkRunnableBase(options)
        return self._runnable

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

    # Method to check if this test has diff'd
    def didDiff(self):
        return self.status.didDiff()

    # Method to check if this test is Initialized
    def isInitialized(self):
        return self.status.isInitialized()

    # Method to check if this test is pending
    def isPending(self):
        return self.status.isPending()

    # Method to check if this test is finished
    def isFinished(self):
        return self.status.isFinished()

    # Method to check if this test is skipped
    def isSkipped(self):
        return self.status.isSkipped()

    # Method to check if this test is silent
    def isSilent(self):
        return self.status.isSilent()

    # Method to check if this test is deleted
    def isDeleted(self):
        return self.status.isDeleted()

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

    # Return boolean on test having redirected output
    def hasRedirectedOutput(self, options):
        return (self.specs.isValid('redirect_output') and self.specs['redirect_output'] == True and self.getProcs(options) > 1)

    # Return a list of redirected output
    def getRedirectedOutputFiles(self, options):
        return [os.path.join(self.getTestDir(), self.name() + '.processor.{}'.format(p)) for p in xrange(self.getProcs(options))]

    # Return active time
    def getActiveTime(self):
        m = re.search(r"Active time=(\S+)", self.getOutput())
        if m != None:
            return m.group(1)

    # Return solve time
    def getSolveTime(self):
        m = re.search(r"solve().*", self.getOutput())
        if m != None:
            return m.group().split()[5]

    # Return active time if available, if not return a comparison of start and end time (RunException tests do not support active time)
    def getTiming(self):
        if self.getActiveTime():
            return self.getActiveTime()
        elif self.getEndTime():
            return self.getEndTime() - self.getStartTime()
        else:
            return 0.0

    # Format the status message to make any caveats easy to read when they are printed
    def formatCaveats(self, options):
        caveats = []
        result = ''

        if self.specs.isValid('caveats'):
            caveats = self.specs['caveats']

        # PASS and DRY_RUN fall into this catagory
        if self.didPass():
            if options.extra_info:
                checks = ['platform', 'compiler', 'petsc_version', 'mesh_mode', 'method', 'library_mode', 'dtk', 'unique_ids']
                for check in checks:
                    if not 'ALL' in self.specs[check]:
                        caveats.append(', '.join(self.specs[check]))
            if len(caveats):
                result = '[' + ', '.join(caveats).upper() + '] ' + self.getSuccessMessage()
            else:
                result = self.getSuccessMessage()

        # FAIL, DIFF and DELETED fall into this catagory
        elif self.didFail() or self.didDiff() or self.isDeleted():
            result = 'FAILED (%s)' % self.getStatusMessage()

        elif self.isSkipped():
            # Include caveats in skipped messages? Usefull to know when a scaled long "RUNNING..." test completes
            # but Exodiff is instructed to 'SKIP' on scaled tests.
            if len(caveats):
                result = '[' + ', '.join(caveats).upper() + '] skipped (' + self.getStatusMessage() + ')'
            else:
                result = 'skipped (' + self.getStatusMessage() + ')'
        else:
            result = self.getStatusMessage()
        return result

    # Print and return formatted current tester status output
    def printResult(self, options):
        # The test has no status to print
        if self.isSilent() or (self.isDeleted() and not options.extra_info):
            caveat_formatted_results = None
        # Print what ever status the tester has at the time
        else:
            if options.verbose or (self.didFail() and not options.quiet):
                output = 'Working Directory: ' + self.getTestDir() + '\nRunning command: ' + self.getCommand(options) + '\n'

                if self.getOutput():
                    output += self.getOutput()
                output = output.replace('\r', '\n')  # replace the carriage returns with newlines
                lines = output.split('\n')

                # Obtain color based on test status
                color = self.getColor()

                if output != '':
                    test_name = util.colorText(self.getTestName()  + ": ", color, colored=options.colored, code=options.code)
                    output = test_name + ("\n" + test_name).join(lines)
                    print(output)

            caveat_formatted_results = self.formatCaveats(options)
            print util.formatResult(self, caveat_formatted_results, self.getTiming(), options)
        return caveat_formatted_results

    # return bool if we want to ignore prereqs requirements
    def skipPrereqs(self, options):
        if options.ignored_caveats:
            caveat_list = [x.lower() for x in options.ignored_caveats.split()]
            if 'all' in options.ignored_caveats or 'prereq' in options.ignored_caveats:
                return True
        return False

    # return bool for output file race conditions
    # NOTE: we return True for exceptions, but they are handled later (because we set a failure status)
    def checkRaceConditions(self, testers, options):
        d = util.DependencyResolver()
        name_to_object = {}
        all_testers = set(testers).union(set([self]))

        for tester in all_testers:
            name_to_object[tester.getTestName()] = tester
            d.insertDependency(tester.getTestName(), tester.getPrereqs())
        try:
            # May fail, which will trigger an exception due cyclic dependencies
            concurrent_tester_sets = d.getSortedValuesSets()
            for concurrent_testers in concurrent_tester_sets:
                output_files_in_dir = set()
                for tester in concurrent_testers:
                    if name_to_object[tester].getTestName() not in self._skipped_tests(testers, options):
                        output_files = name_to_object[tester].getOutputFiles()
                        duplicate_files = output_files_in_dir.intersection(output_files)
                        if len(duplicate_files):
                            return False
                        output_files_in_dir.update(output_files)
        except:
            self.setStatus('Cyclic or Invalid Dependency Detected!', self.bucket_fail)
        return True

    # return a set of finished non-passing or will be skipped tests
    def _skipped_tests(self, testers, options):
        skipped_failed = set([])
        for tester in testers:
            if (tester.isFinished() and not tester.didPass()) \
               or not tester.getRunnable(options) \
               or not tester.shouldExecute():
                skipped_failed.add(tester.getTestName())
        return skipped_failed

    # This is the base level runnable check common to all Testers.  DO NOT override
    # this method in any of your derived classes.  Instead see "checkRunnable"
    def checkRunnableBase(self, options):
        reasons = {}
        checks = options._checks

        # If the something has already deemed this test a failure, return now
        if self.didFail():
            return False

        # If --dry-run set the test status to pass and DO NOT return.
        # This will allow additional checks to perform and report tests
        # that would normally be skipped (and return as False).
        if options.dry_run:
            self.success_message = 'DRY RUN'
            self.setStatus(self.success_message, self.bucket_success)

        # Check if we only want to run failed tests
        if options.failed_tests:
            if self.specs['test_name'] not in options._test_list:
                self.setStatus('not failed', self.bucket_silent)
                return False

        # Check if we only want to run syntax tests
        if options.check_input:
            if not self.specs['check_input']:
                self.setStatus('not check_input', self.bucket_silent)
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

        # Short circuit method and run this test if we are ignoring all caveats
        if options.ignored_caveats == 'all':
            # Still, we should abide by the derived classes
            return self.checkRunnable(options)

        # Check for deleted tests
        if self.specs.isValid('deleted'):
            reasons['deleted'] = 'deleted (' + self.specs['deleted'] + ')'

        # Check for skipped tests
        if self.specs.type('skip') is bool and self.specs['skip']:
            # Backwards compatible (no reason)
            reasons['skip'] = 'no reason'
        elif self.specs.type('skip') is not bool and self.specs.isValid('skip'):
            reasons['skip'] = self.specs['skip']
        # If were testing for SCALE_REFINE, then only run tests with a SCALE_REFINE set
        elif (options.store_time or options.scaling) and self.specs['scale_refine'] == 0:
            self.setStatus('silent', self.bucket_silent)
            return False
        # If we're testing with valgrind, then skip tests that require parallel or threads or don't meet the valgrind setting
        elif options.valgrind_mode != '':
            tmp_reason = ''
            if self.specs['valgrind'].upper() == 'NONE':
                tmp_reason = 'Valgrind==NONE'
            elif self.specs['valgrind'].upper() == 'HEAVY' and options.valgrind_mode.upper() == 'NORMAL':
                tmp_reason = 'Valgrind==HEAVY'
            elif self.specs['min_parallel'] > 1 or self.specs['min_threads'] > 1:
                tmp_reason = 'Valgrind requires serial'
            if tmp_reason != '':
                reasons['valgrind'] = tmp_reason
        # If we're running in recover mode skip tests that have recover = false
        elif options.enable_recover and self.specs['recover'] == False:
            reasons['recover'] = 'NO RECOVER'

        # Check for PETSc versions
        (petsc_status, logic_reason, petsc_version) = util.checkPetscVersion(checks, self.specs)
        if not petsc_status:
            reasons['petsc_version'] = 'using PETSc ' + str(checks['petsc_version']) + ' REQ: ' + logic_reason + ' ' + petsc_version

        # Check for SLEPc versions
        (slepc_status, logic_reason, slepc_version) = util.checkSlepcVersion(checks, self.specs)
        if not slepc_status and len(self.specs['slepc_version']) != 0:
            if slepc_version != None:
                reasons['slepc_version'] = 'using SLEPc ' + str(checks['slepc_version']) + ' REQ: ' + logic_reason + ' ' + slepc_version
            elif slepc_version == None:
                reasons['slepc_version'] = 'SLEPc is not installed'

        # PETSc and SLEPc is being explicitly checked above
        local_checks = ['platform', 'compiler', 'mesh_mode', 'method', 'library_mode', 'dtk', 'unique_ids', 'vtk', 'tecplot', \
                        'petsc_debug', 'curl', 'tbb', 'superlu', 'cxx11', 'asio', 'unique_id', 'slepc', 'petsc_version_release', 'boost']
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

        # Check for heavy tests
        if options.all_tests or options.heavy_tests:
            if not self.specs['heavy'] and options.heavy_tests:
                reasons['heavy'] = 'NOT HEAVY'
        elif self.specs['heavy']:
            reasons['heavy'] = 'HEAVY'

        # Check for positive scale refine values when using store timing options
        if self.specs['scale_refine'] == 0 and options.store_time:
            reasons['scale_refine'] = 'scale_refine==0 store_time=True'

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

        # Check to make sure required submodules are initialized
        for var in self.specs['required_submodule']:
            if var not in checks["submodules"]:
                reasons['required_submodule'] = '%s submodule not initialized' % var

        # Check to make sure environment variable exists
        for var in self.specs['env_vars']:
            if not os.environ.has_key(var):
                reasons['env_vars'] = 'ENV VAR NOT SET'

        # Check for display
        if self.specs['display_required'] and not os.getenv('DISPLAY', False):
            reasons['display_required'] = 'NO DISPLAY'

        # Remove any matching user supplied caveats from accumulated checkRunnable caveats that
        # would normally produce a skipped test.
        caveat_list = set()
        if options.ignored_caveats:
            caveat_list = set([x.lower() for x in options.ignored_caveats.split()])

        if len(set(reasons.keys()) - caveat_list) > 0:
            tmp_reason = []
            for key, value in reasons.iteritems():
                if key.lower() not in caveat_list:
                    tmp_reason.append(value)

            # Format joined reason to better fit on the screen
            if len(', '.join(tmp_reason)) >= util.TERM_COLS - (len(self.specs['test_name'])+21):
                flat_reason = (', '.join(tmp_reason))[:(util.TERM_COLS - (len(self.specs['test_name'])+24))] + '...'
            else:
                flat_reason = ', '.join(tmp_reason)

            # If the test is deleted we still need to treat this differently
            if 'deleted' in reasons.keys():
                self.setStatus(flat_reason, self.bucket_deleted)
            else:
                self.setStatus(flat_reason, self.bucket_skip)
            return False

        # Check the return values of the derived classes
        self._runnable = self.checkRunnable(options)
        return self._runnable
