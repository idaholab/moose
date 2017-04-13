import re, os, sys, time
from Tester import Tester
import util
from RunParallel import RunParallel # For TIMEOUT value

class RunApp(Tester):

    @staticmethod
    def validParams():
        params = Tester.validParams()
        params.addParam('input',              "The input file to use for this test.")
        params.addParam('test_name',          "The name of the test - populated automatically")
        params.addParam('input_switch', '-i', "The default switch used for indicating an input to the executable")
        params.addParam('errors',             ['ERROR', 'command not found', 'erminate called after throwing an instance of'], "The error messages to detect a failed run")
        params.addParam('expect_out',         "A regular expression that must occur in the input in order for the test to be considered passing.")
        params.addParam('match_literal', False, "Treat expect_out as a string not a regular expression.")
        params.addParam('absent_out',         "A regular expression that must be *absent* from the output for the test to pass.")
        params.addParam('should_crash', False, "Inidicates that the test is expected to crash or otherwise terminate early")
        params.addParam('executable_pattern', "A test that only runs if the exectuable name matches the given pattern")
        params.addParam('delete_output_before_running',  True, "Delete pre-existing output files before running test. Only set to False if you know what you're doing!")
        params.addParam('delete_output_folders', True, "Delete output folders before running")

        # RunApp can also run arbitrary commands. If the "command" parameter is supplied
        # it'll be used in lieu of building up the command automatically
        params.addParam('command',            "The command line to execute for this test.")

        params.addParam('walltime',           "The max time as pbs understands it")
        params.addParam('job_name',           "The test name as pbs understands it")
        params.addParam('no_copy',            "The tests file as pbs understands it")

        # Parallel/Thread testing
        params.addParam('max_parallel', 1000, "Maximum number of MPI processes this test can be run with      (Default: 1000)")
        params.addParam('min_parallel',    1, "Minimum number of MPI processes that this test can be run with (Default: 1)")
        params.addParam('max_threads',    16, "Max number of threads (Default: 16)")
        params.addParam('min_threads',     1, "Min number of threads (Default: 1)")
        params.addParam('allow_warnings',   False, "If the test harness is run --error warnings become errors, setting this to true will disable this an run the test without --error");
        params.addParam('redirect_output',  False, "Redirect stdout to files. Neccessary when expecting an error when using parallel options")

        params.addParamWithType('allow_deprecated_until', type(time.localtime()), "A test that only runs if current date is less than specified date")

        # Valgrind
        params.addParam('valgrind', 'NORMAL', "Set to (NONE, NORMAL, HEAVY) to determine which configurations where valgrind will run.")

        return params

    def __init__(self, name, params):
        Tester.__init__(self, name, params)
        if os.environ.has_key("MOOSE_MPI_COMMAND"):
            self.mpi_command = os.environ['MOOSE_MPI_COMMAND']
            self.force_mpi = True
        else:
            self.mpi_command = 'mpiexec'
            self.force_mpi = False

        # Handle the special allow_deprecated_until parameter
        if params.isValid('allow_deprecated_until') and params['allow_deprecated_until'] > time.localtime():
            self.specs['cli_args'].append('--allow-deprecated')

        # Make sure that either input or command is supplied
        if not (params.isValid('input') or params.isValid('command')):
            raise Exception('Either "input" or "command" must be supplied for a RunApp test')

    def getInputFile(self):
        return self.specs['input'].strip()

    def checkRunnable(self, options):
        if options.enable_recover:
            if self.specs.isValid('expect_out') or self.specs.isValid('absent_out') or self.specs['should_crash'] == True:
                self.setStatus('expect_out RECOVER', self.bucket_skip)
                return False

        if self.specs.isValid('executable_pattern') and re.search(self.specs['executable_pattern'], self.specs['executable']) == None:
            self.setStatus('EXECUTABLE PATTERN', self.bucket_skip)
            return False

        return True

    def getThreads(self, options):
        #Set number of threads to be used lower bound
        nthreads = max(options.nthreads, int(self.specs['min_threads']))
        #Set number of threads to be used upper bound
        nthreads = min(nthreads, int(self.specs['max_threads']))
        return nthreads

    def getProcs(self, options):
        if options.parallel == None:
            default_ncpus = 1
        else:
            default_ncpus = options.parallel

        # Raise the floor
        ncpus = max(default_ncpus, int(self.specs['min_parallel']))
        # Lower the ceiling
        ncpus = min(ncpus, int(self.specs['max_parallel']))
        return ncpus

    def getCommand(self, options):
        specs = self.specs

        # Just return an arbitrary command if one is supplied
        if specs.isValid('command'):
            return os.path.join(specs['test_dir'], specs['command']) + ' ' + ' '.join(specs['cli_args'])

        # Create the command line string to run
        command = ''

        # Check for built application
        if not options.dry_run and not os.path.exists(specs['executable']):
            print os.getcwd()
            print 'Application not found: ' + str(specs['executable'])
            sys.exit(1)

        if (options.parallel_mesh or options.distributed_mesh) and ('--parallel-mesh' not in specs['cli_args'] or '--distributed-mesh' not in specs['cli_args']):
            # The user has passed the parallel-mesh option to the test harness
            # and it is NOT supplied already in the cli-args option
            specs['cli_args'].append('--distributed-mesh')

        if options.error and '--error' not in specs['cli_args'] and not specs["allow_warnings"]:
            # The user has passed the error option to the test harness
            # and it is NOT supplied already in the cli-args option\
            specs['cli_args'].append('--error')

        if options.error_unused and '--error-unused' not in specs['cli_args'] and '--warn-unused' not in specs['cli_args'] and not specs["allow_warnings"]:
            # The user has passed the error-unused option to the test harness
            # and it is NOT supplied already in the cli-args option
            # also, neither is the conflicting option "warn-unused"
            specs['cli_args'].append('--error-unused')

        if self.getCheckInput():
            specs['cli_args'].append('--check-input')

        timing_string = ' '
        if options.timing:
            specs['cli_args'].append('--timing')
            specs['cli_args'].append('Outputs/print_perf_log=true')

        if options.colored == False:
            specs['cli_args'].append('--color off')

        if options.cli_args:
            specs['cli_args'].insert(0, options.cli_args)

        if options.scaling and specs['scale_refine'] > 0:
            specs['cli_args'].insert(0, ' -r ' + str(specs['scale_refine']))

        # The test harness should never use GDB backtraces: they don't
        # work well when dozens of expect_err jobs run at the same time.
        specs['cli_args'].append('--no-gdb-backtrace')

        # Get the number of processors and threads the Tester requires
        ncpus = self.getProcs(options)
        nthreads = self.getThreads(options)

        if options.parallel == None:
            default_ncpus = 1
        else:
            default_ncpus = options.parallel

        if specs['redirect_output'] and ncpus > 1:
            specs['cli_args'].append('--keep-cout --redirect-output ' + self.name())

        caveats = []
        if nthreads > options.nthreads:
            caveats.append('min_threads=' + str(nthreads))
        elif nthreads < options.nthreads:
            caveats.append('max_threads=' + str(nthreads))
        # TODO: Refactor this caveats business
        if ncpus > default_ncpus:
            caveats.append('min_cpus=' + str(ncpus))
        elif ncpus < default_ncpus:
            caveats.append('max_cpus=' + str(ncpus))

        if len(caveats) > 0:
            self.specs['caveats'] = caveats

        if self.force_mpi or options.parallel or ncpus > 1 or nthreads > 1:
            command = self.mpi_command + ' -n ' + str(ncpus) + ' ' + specs['executable'] + ' --n-threads=' + str(nthreads) + ' ' + specs['input_switch'] + ' ' + specs['input'] + ' ' +  ' '.join(specs['cli_args'])
        elif options.valgrind_mode.upper() == specs['valgrind'].upper() or options.valgrind_mode.upper() == 'HEAVY' and specs['valgrind'].upper() == 'NORMAL':
            command = 'valgrind --suppressions=' + os.path.join(specs['moose_dir'], 'python', 'TestHarness', 'suppressions', 'errors.supp') + ' --leak-check=full --tool=memcheck --dsymutil=yes --track-origins=yes --demangle=yes -v ' + specs['executable'] + ' ' + specs['input_switch'] + ' ' + specs['input'] + ' ' + ' '.join(specs['cli_args'])
        else:
            command = specs['executable'] + timing_string + specs['input_switch'] + ' ' + specs['input'] + ' ' + ' '.join(specs['cli_args'])

        if options.pbs:
            return self.getPBSCommand(options)

        return command

    def getPBSCommand(self, options):
        if options.parallel == None:
            default_ncpus = 1
        else:
            default_ncpus = options.parallel

        # Raise the floor
        ncpus = max(default_ncpus, int(self.specs['min_parallel']))
        # Lower the ceiling
        ncpus = min(ncpus, int(self.specs['max_parallel']))

        # Set number of threads to be used lower bound
        nthreads = max(options.nthreads, int(self.specs['min_threads']))
        # Set number of threads to be used upper bound
        nthreads = min(nthreads, int(self.specs['max_threads']))

        extra_args = ''
        if options.parallel or ncpus > 1 or nthreads > 1:
            extra_args = ' --n-threads=' + str(nthreads) + ' ' + ' '.join(self.specs['cli_args'])

        timing_string = ' '
        if options.timing:
            self.specs['cli_args'].append('--timing')
            self.specs['cli_args'].append('Outputs/print_perf_log=true')

        # Append any extra args to the cluster_launcher
        if extra_args != '':
            self.specs['cli_args'] = extra_args
        else:
            self.specs['cli_args'] = ' '.join(self.specs['cli_args'])
        self.specs['cli_args'] = "'" + self.specs['cli_args'].strip() + "'"

        # Open our template. This should probably be done at the same time as cluster_handle.
        template_script = open(os.path.join(self.specs['moose_dir'], 'python', 'TestHarness', 'pbs_template.i'), 'r')
        content = template_script.read()
        template_script.close()

        # Convert MAX_TIME to hours:minutes for walltime use
        hours = int(int(self.specs['max_time']) / 3600)
        minutes = int(int(self.specs['max_time']) / 60) % 60
        self.specs['walltime'] = '{0:02d}'.format(hours) + ':' + '{0:02d}'.format(minutes) + ':00'

        # Truncate JOB_NAME. PBS can only accept 13 character (6 characters from test name + _## (test serial number) + _### (serialized number generated by cluster_launcher) = the 13 character limit)
        self.specs['job_name'] = self.specs['input'][:6] + '_' + str(options.test_serial_number).zfill(2)
        self.specs['job_name'] = self.specs['job_name'].replace('.', '')
        self.specs['job_name'] = self.specs['job_name'].replace('-', '')

        # Convert TEST_NAME to input tests file name (normally just 'tests')
        self.specs['no_copy'] = options.input_file_name

        # Are we using the PBS Emulator? Make this param valid if so.
        # Add the substitution string here so it is not visable to the user
        self.specs.addStringSubParam('pbs_stdout', 'PBS_STDOUT', "Save stdout to this location")
        self.specs.addStringSubParam('pbs_stderr', 'PBS_STDERR', "Save stderr to this location")
        if options.PBSEmulator:
            self.specs['pbs_stdout'] = 'pbs_stdout = PBS_EMULATOR'
            self.specs['pbs_stderr'] = 'pbs_stderr = PBS_EMULATOR'
        else:
            # The PBS Emulator fails when using the PROJECT argument (#PBS -P <project name>)
            self.specs.addStringSubParam('pbs_project', 'PBS_PROJECT', "Identify this job submission with this project")
            self.specs['pbs_project'] = 'pbs_project = %s' % (options.pbs_project)


        # Do all of the replacements for the valid parameters
        for spec in self.specs.valid_keys():
            if spec in self.specs.substitute:
                self.specs[spec] = self.specs.substitute[spec].replace(spec.upper(), self.specs[spec])
            content = content.replace('<' + spec.upper() + '>', str(self.specs[spec]))

        # Make sure we strip out any string substitution parameters that were not supplied
        for spec in self.specs.substitute_keys():
            if not self.specs.isValid(spec):
                content = content.replace('<' + spec.upper() + '>', '')

        # Write the cluster_launcher input file
        options.cluster_handle.write(content + '\n')

        return os.path.join(self.specs['moose_dir'], 'scripts', 'cluster_launcher.py') + ' ' + options.pbs + '.cluster'


    def processResults(self, moose_dir, retcode, options, output):
        reason = ''
        specs = self.specs
        if specs.isValid('expect_out'):
            if specs['match_literal']:
                have_expected_out = util.checkOutputForLiteral(output, specs['expect_out'])
            else:
                have_expected_out = util.checkOutputForPattern(output, specs['expect_out'])
            if (not have_expected_out):
                reason = 'EXPECTED OUTPUT MISSING'

        if reason == '' and specs.isValid('absent_out'):
            have_absent_out = util.checkOutputForPattern(output, specs['absent_out'])
            if (have_absent_out):
                reason = 'OUTPUT NOT ABSENT'

        if reason == '':
            # We won't pay attention to the ERROR strings if EXPECT_ERR is set (from the derived class)
            # since a message to standard error might actually be a real error.  This case should be handled
            # in the derived class.
            if options.valgrind_mode == '' and not specs.isValid('expect_err') and len( filter( lambda x: x in output, specs['errors'] ) ) > 0:
                reason = 'ERRMSG'
            elif retcode == RunParallel.TIMEOUT:
                reason = 'TIMEOUT'
            elif retcode == 0 and specs['should_crash'] == True:
                reason = 'NO CRASH'
            elif retcode != 0 and specs['should_crash'] == False:
                reason = 'CRASH'
            # Valgrind runs
            elif retcode == 0 and self.shouldExecute() and options.valgrind_mode != '' and 'ERROR SUMMARY: 0 errors' not in output:
                reason = 'MEMORY ERROR'
            # PBS runs
            elif retcode == 0 and options.pbs and 'command not found' in output:
                reason = 'QSUB NOT FOUND'

        # Populate the bucket
        if reason != '':
            self.setStatus(reason, self.bucket_fail)
        else:
            self.setStatus(self.success_message, self.bucket_success)

        return output
