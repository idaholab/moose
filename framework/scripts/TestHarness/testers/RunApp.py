from options import *
import re, os
from Tester import Tester
from RunParallel import RunParallel # For TIMEOUT value

class RunApp(Tester):

  def getValidParams():
    params = Tester.getValidParams()
    params.addRequiredParam('input',      "The input file to use for this test.")
    params.addParam('test_name',          "The name of the test - populated automatically")
    params.addParam('cli_args',       [], "Additional arguments to be passed to the test.")
    params.addParam('input_switch', '-i', "The default switch used for indicating an input to the executable")
    params.addParam('errors',             ['ERROR', 'command not found', 'erminate called after throwing an instance of'], "The error messages to detect a failed run")
    params.addParam('expect_out',         "A regular expression that must occur in the input in order for the test to be considered passing.")
    params.addParam('should_crash',False, "Inidicates that the test is expected to crash or otherwise terminate early")

    params.addParam('walltime',           "The max time as pbs understands it")
    params.addParam('job_name',           "The test name as pbs understands it")
    params.addParam('no_copy',            "The tests file as pbs understands it")

    # Parallel/Thread testing
    params.addParam('max_parallel', 1000, "Maximum number of MPI processes this test can be run with      (Default: 1000)")
    params.addParam('min_parallel',    1, "Minimum number of MPI processes that this test can be run with (Default: 1)")
    params.addParam('max_threads',    16, "Max number of threads (Default: 16)")
    params.addParam('min_threads',     1, "Min number of threads (Default: 1)")
    params.addParam('scale_refine',    0, "The number of refinements to do when scaling")

    # Valgrind
    params.addParam('no_valgrind', False, "Set to True to skip test when running with --valgrind")

    return params
  getValidParams = staticmethod(getValidParams)

  def __init__(self, name, params):
    Tester.__init__(self, name, params)

  def checkRunnable(self, options):
    if options.enable_recover:
      if self.specs.isValid('expect_out') or self.specs[SHOULD_CRASH] == True:
        reason = 'skipped (expect_out RECOVER)'
        return (False, reason)
    return (True, '')

  def getCommand(self, options):
    # Create the command line string to run
    command = ''

    specs = self.specs

    if options.parallel == None:
      default_ncpus = 1
    else:
      default_ncpus = options.parallel

    timing_string = ' '
    if options.timing:
      timing_string = ' Output/perf_log=true '

    # Raise the floor
    ncpus = max(default_ncpus, int(specs[MIN_PARALLEL]))
    # Lower the ceiling
    ncpus = min(ncpus, int(specs[MAX_PARALLEL]))

    #Set number of threads to be used lower bound
    nthreads = max(options.nthreads, int(specs[MIN_THREADS]))
    #Set number of threads to be used upper bound
    nthreads = min(nthreads, int(specs[MAX_THREADS]))

    if nthreads > options.nthreads:
      self.specs['CAVEATS'] = ['MIN_THREADS=' + str(nthreads)]
    elif nthreads < options.nthreads:
      self.specs['CAVEATS'] = ['MAX_THREADS=' + str(nthreads)]
    # TODO: Refactor this caveats business
    if ncpus > default_ncpus:
      self.specs['CAVEATS'] = ['MIN_CPUS=' + str(ncpus)]
    elif ncpus < default_ncpus:
      self.specs['CAVEATS'] = ['MAX_CPUS=' + str(ncpus)]
    if options.parallel or ncpus > 1 or nthreads > 1:
      command = 'mpiexec -host localhost -n ' + str(ncpus) + ' ' + specs[EXECUTABLE] + ' --n-threads=' + str(nthreads) + ' ' + specs[INPUT_SWITCH] + ' ' + specs[INPUT] + ' ' +  ' '.join(specs[CLI_ARGS])
    elif options.enable_valgrind and not specs[NO_VALGRIND]:
      command = 'valgrind --suppressions=' + specs[MOOSE_DIR] + 'scripts/TestHarness/suppressions/errors.supp --leak-check=full --tool=memcheck --dsymutil=yes --track-origins=yes -v ' + specs[EXECUTABLE] + ' ' + specs[INPUT_SWITCH] + ' ' + specs[INPUT] + ' ' + ' '.join(specs[CLI_ARGS])
    else:
      command = specs[EXECUTABLE] + timing_string + specs[INPUT_SWITCH] + ' ' + specs[INPUT] + ' ' + ' '.join(specs[CLI_ARGS])

    if options.scaling and specs[SCALE_REFINE] > 0:
      command += ' -r ' + str(specs[SCALE_REFINE])

    if options.cli_args:
      command += ' ' + options.cli_args

    if options.pbs:
      return self.getPBSCommand(options)
    return command

  def getPBSCommand(self, options):
    if options.parallel == None:
      default_ncpus = 1
    else:
      default_ncpus = options.parallel

    # Raise the floor
    ncpus = max(default_ncpus, int(self.specs[MIN_PARALLEL]))
    # Lower the ceiling
    ncpus = min(ncpus, int(self.specs[MAX_PARALLEL]))

    #Set number of threads to be used lower bound
    nthreads = max(options.nthreads, int(self.specs[MIN_THREADS]))
    #Set number of threads to be used upper bound
    nthreads = min(nthreads, int(self.specs[MAX_THREADS]))

    extra_args = ''
    if options.parallel or ncpus > 1 or nthreads > 1:
      extra_args = ' --n-threads=' + str(nthreads) + ' ' + ' '.join(self.specs[CLI_ARGS])

    # Append any extra args to the cluster_launcher
    if extra_args != '':
      self.specs[CLI_ARGS] = extra_args
    else:
      self.specs[CLI_ARGS] = ' '.join(self.specs[CLI_ARGS])
    self.specs[CLI_ARGS] = self.specs[CLI_ARGS].strip()

    # Open our template. This should probably be done at the same time as cluster_handle.
    template_script = open(self.specs[MOOSE_DIR] + 'scripts/TestHarness/pbs_template.i', 'r')
    content = template_script.read()
    template_script.close()

    # Convert MAX_TIME to hours:minutes for walltime use
    hours = int(int(self.specs[MAX_TIME]) / 3600)
    minutes = int(int(self.specs[MAX_TIME]) / 60) % 60
    self.specs['walltime'] = '{:02,.0f}'.format(hours) + ':' + '{:02,.0f}'.format(minutes) + ':00'

    # Truncate JOB_NAME, as PBS can only except 13 character -N (6 characters from test name + _TH (TestHarness) + _### (serialized number generated by cluster_launcher) = the 13 character limit)
    self.specs['job_name'] = self.specs[INPUT][:6] + '_TH'
    self.specs['job_name'] = self.specs['job_name'].replace('.', '')
    self.specs['job_name'] = self.specs['job_name'].replace('-', '')

    # Convert TEST_NAME to input tests file name (normally just 'tests')
    self.specs['no_copy'] = options.input_file_name

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

    return self.specs[MOOSE_DIR] + 'scripts/cluster_launcher.py tests.cluster'


  def processResults(self, moose_dir, retcode, options, output):
    reason = ''
    specs = self.specs

    # PBS runs
    if options.pbs != None:
      if retcode == 0 and 'command not found' in output:
        reason = 'QSUB NOT FOUND'
    # Everything else
    else:
      if specs.isValid(EXPECT_OUT):
        out_ok = self.checkOutputForPattern(output, specs[EXPECT_OUT])
        if (out_ok and retcode != 0):
          reason = 'OUT FOUND BUT CRASH'
        elif (not out_ok):
          reason = 'NO EXPECTED OUT'
      if reason == '':
        # We won't pay attention to the ERROR strings if EXPECT_ERR is set (from the derived class)
        # since a message to standard error might actually be a real error.  This case should be handled
        # in the derived class.
        if not options.enable_valgrind and not specs.isValid(EXPECT_ERR) and len( filter( lambda x: x in output, specs[ERRORS] ) ) > 0:
          reason = 'ERRMSG'
        elif retcode == RunParallel.TIMEOUT:
          reason = 'TIMEOUT'
        elif retcode == 0 and specs[SHOULD_CRASH] == True:
          reason = 'NO CRASH'
        elif retcode != 0 and specs[SHOULD_CRASH] == False:
          reason = 'CRASH'
        # Valgrind runs
        elif retcode == 0 and options.enable_valgrind and not specs[NO_VALGRIND] and 'ERROR SUMMARY: 0 errors' not in output:
          reason = 'MEMORY ERROR'

    return (reason, output)

  def checkOutputForPattern(self, output, re_pattern):
    if re.search(re_pattern, output, re.MULTILINE | re.DOTALL) == None:
      return False
    else:
      return True
