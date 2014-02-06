#from options import *
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
    params.addParam('valgrind', 'NORMAL', "Set to (NONE, NORMAL, HEAVY) to determine which configurations where valgrind will run.")

    return params
  getValidParams = staticmethod(getValidParams)

  def __init__(self, name, params):
    Tester.__init__(self, name, params)

  def checkRunnable(self, options):
    if options.enable_recover:
      if self.specs.isValid('expect_out') or self.specs['should_crash'] == True:
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
    ncpus = max(default_ncpus, int(specs['min_parallel']))
    # Lower the ceiling
    ncpus = min(ncpus, int(specs['max_parallel']))

    #Set number of threads to be used lower bound
    nthreads = max(options.nthreads, int(specs['min_threads']))
    #Set number of threads to be used upper bound
    nthreads = min(nthreads, int(specs['max_threads']))

    if nthreads > options.nthreads:
      self.specs['caveats'] = ['min_threads=' + str(nthreads)]
    elif nthreads < options.nthreads:
      self.specs['caveats'] = ['max_threads=' + str(nthreads)]
    # TODO: Refactor this caveats business
    if ncpus > default_ncpus:
      self.specs['caveats'] = ['min_cpus=' + str(ncpus)]
    elif ncpus < default_ncpus:
      self.specs['caveats'] = ['max_cpus=' + str(ncpus)]
    if options.parallel or ncpus > 1 or nthreads > 1:
      command = 'mpiexec -host localhost -n ' + str(ncpus) + ' ' + specs['executable'] + ' --n-threads=' + str(nthreads) + ' ' + specs['input_switch'] + ' ' + specs['input'] + ' ' +  ' '.join(specs['cli_args'])
    elif options.valgrind_mode == specs['valgrind'] or options.valgrind_mode == 'HEAVY' and specs[VALGRIND] == 'NORMAL':
      command = 'valgrind --suppressions=' + specs['moose_dir'] + 'scripts/TestHarness/suppressions/errors.supp --leak-check=full --tool=memcheck --dsymutil=yes --track-origins=yes -v ' + specs['executable'] + ' ' + specs['input_switch'] + ' ' + specs['input'] + ' ' + ' '.join(specs['cli_args'])
    else:
      command = specs['executable'] + timing_string + specs['input_switch'] + ' ' + specs['input'] + ' ' + ' '.join(specs['cli_args'])

    if options.scaling and specs['scale_refine'] > 0:
      command += ' -r ' + str(specs['scale_refine'])

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
    ncpus = max(default_ncpus, int(self.specs['min_parallel']))
    # Lower the ceiling
    ncpus = min(ncpus, int(self.specs['max_parallel']))

    #Set number of threads to be used lower bound
    nthreads = max(options.nthreads, int(self.specs['min_threads']))
    #Set number of threads to be used upper bound
    nthreads = min(nthreads, int(self.specs['max_threads']))

    extra_args = ''
    if options.parallel or ncpus > 1 or nthreads > 1:
      extra_args = ' --n-threads=' + str(nthreads) + ' ' + ' '.join(self.specs['cli_args'])

    # Append any extra args to the cluster_launcher
    if extra_args != '':
      self.specs['cli_args'] = extra_args
    else:
      self.specs['cli_args'] = ' '.join(self.specs['cli_args'])
    self.specs['cli_args'] = self.specs['cli_args'].strip()

    # Open our template. This should probably be done at the same time as cluster_handle.
    template_script = open(self.specs['moose_dir'] + 'scripts/TestHarness/pbs_template.i', 'r')
    content = template_script.read()
    template_script.close()

    # Convert MAX_TIME to hours:minutes for walltime use
    hours = int(int(self.specs['max_time']) / 3600)
    minutes = int(int(self.specs['max_time']) / 60) % 60
    self.specs['walltime'] = '{:02,.0f}'.format(hours) + ':' + '{:02,.0f}'.format(minutes) + ':00'

    # Truncate JOB_NAME, as PBS can only accept 13 character -N (6 characters from test name + _TH (TestHarness) + _### (serialized number generated by cluster_launcher) = the 13 character limit)
    self.specs['job_name'] = self.specs['input'][:6] + '_TH'
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

    return self.specs['moose_dir'] + 'scripts/cluster_launcher.py tests.cluster'


  def processResults(self, moose_dir, retcode, options, output):
    reason = ''
    specs = self.specs
    if specs.isValid('expect_out'):
      out_ok = self.checkOutputForPattern(output, specs['expect_out'])
      if (out_ok and retcode != 0):
        reason = 'OUT FOUND BUT CRASH'
      elif (not out_ok):
        reason = 'NO EXPECTED OUT'
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
      elif retcode == 0 and options.valgrind_mode != '' and 'ERROR SUMMARY: 0 errors' not in output:
        reason = 'MEMORY ERROR'
      # PBS runs
      elif retcode == 0 and options.pbs and 'command not found' in output:
        reason = 'QSUB NOT FOUND'

    return (reason, output)

  def checkOutputForPattern(self, output, re_pattern):
    if re.search(re_pattern, output, re.MULTILINE | re.DOTALL) == None:
      return False
    else:
      return True
