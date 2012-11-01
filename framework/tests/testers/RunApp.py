from options import *
from Tester import Tester

class RunApp(Tester):
  def __init__(self, klass, specs):
    Tester.__init__(self, klass, specs)

  def prepare(self):
    return

  def getCommand(self, options):
    # Create the command line string to run
    command = ''

    specs = self.specs

    # Raise the floor
    ncpus = max(options.parallel, int(specs[MIN_PARALLEL]))
    # Lower the ceiling
    ncpus = min(ncpus, int(specs[MAX_PARALLEL]))

    #Set number of threads to be used lower bound
    nthreads = max(options.nthreads, int(specs[MIN_THREADS]))
    #Set number of threads to be used upper bound
    nthreads = min(nthreads, int(specs[MAX_THREADS]))

    if nthreads > options.nthreads:
      specs['CAVEATS'] = ['MIN_THREADS=' + str(nthreads)]
    elif nthreads < options.nthreads:
      specs['CAVEATS'] = ['MAX_THREADS=' + str(nthreads)]
    # TODO: Refactor this caveats business
    if ncpus > options.parallel:
      specs['CAVEATS'] = ['MIN_CPUS=' + str(ncpus)]
    elif ncpus < options.parallel:
      specs['CAVEATS'] = ['MAX_CPUS=' + str(ncpus)]
    if ncpus > 1 or nthreads > 1:
      command = 'mpiexec -host ' + specs[HOSTNAME] + ' -n ' + str(ncpus) + ' ' + specs[EXECUTABLE] + ' --n-threads=' + str(nthreads) + ' -i ' + specs[INPUT] + ' ' +  ' '.join(specs[CLI_ARGS])
    elif options.enable_valgrind and not specs[NO_VALGRIND]:
      command = 'valgrind --tool=memcheck --dsymutil=yes --track-origins=yes -v ' + specs[EXECUTABLE] + ' -i ' + specs[INPUT] + ' ' + ' '.join(specs[CLI_ARGS])
    else:
      command = specs[EXECUTABLE] + ' -i ' + specs[INPUT] + ' ' + ' '.join(specs[CLI_ARGS])

    if options.scaling and specs[SCALE_REFINE] > 0:
      command += ' -r ' + str(specs[SCALE_REFINE])
    return command

  def processResults(self, moose_dir, retcode, output):
    return

