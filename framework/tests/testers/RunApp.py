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

  def getCommand(self, options):
    # Create the command line string to run
    command = ''

    specs = self.specs

    if options.parallel == None:
      default_ncpus = 1
    else:
      default_ncpus = options.parallel

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
      command = 'valgrind --tool=memcheck --dsymutil=yes --track-origins=yes -v ' + specs[EXECUTABLE] + ' ' + specs[INPUT_SWITCH] + ' ' + specs[INPUT] + ' ' + ' '.join(specs[CLI_ARGS])
    else:
      command = specs[EXECUTABLE] + ' ' + specs[INPUT_SWITCH] + ' ' + specs[INPUT] + ' ' + ' '.join(specs[CLI_ARGS])

    if options.scaling and specs[SCALE_REFINE] > 0:
      command += ' -r ' + str(specs[SCALE_REFINE])

    return command

  def processResults(self, moose_dir, retcode, options, output):
    reason = ''
    specs = self.specs

    if specs.isValid(EXPECT_OUT):
      out_ok = self.checkOutputForPattern(output, specs[EXPECT_OUT])
      if (out_ok and retcode != 0):
        reason = 'OUT FOUND BUT CRASH'
      elif (not out_ok):
        reason = 'NO EXPECTED OUT'
    elif (options.enable_valgrind and retcode == 0) and not specs[NO_VALGRIND]:
      if 'ERROR SUMMARY: 0 errors' not in output:
        reason = 'MEMORY ERROR'
    else:
      # Check the general error message and program crash possibilities
      if len( filter( lambda x: x in output, specs[ERRORS] ) ) > 0:
        reason = 'ERRMSG'
      elif retcode == RunParallel.TIMEOUT:
        reason = 'TIMEOUT'
      elif retcode != 0 and not specs[SHOULD_CRASH]:
        reason = 'CRASH'
      elif retcode > 0 and specs[SHOULD_CRASH]:
        reason = 'NO CRASH'

    return (reason, output)


  def checkOutputForPattern(self, output, re_pattern):
    if re.search(re_pattern, output, re.MULTILINE | re.DOTALL) == None:
      return False
    else:
      return True

