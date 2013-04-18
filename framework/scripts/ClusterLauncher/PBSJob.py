from InputParameters import InputParameters
from Job import Job

import os, sys, subprocess

class PBSJob(Job):
  def getValidParams():
    params = Job.getValidParams()

    params.addRequiredParam('chunks', "The number of PBS chunks.")
    # Only one of either of the next two paramteres can be specified
    params.addParam('mpi_procs', "The number of MPI processes per chunk.")
    params.addParam('total_mpi_procs', "The total number of MPI processes to use divided evenly among chunks.")

    params.addParam('place', 'scatter:excl', "The PBS job placement scheme to use.")
    params.addParam('walltime', '4:00:00', "The requested walltime for this job.")

    params.addStringSubParam('threads', '--n-threads=THREADS', "The number of threads to run per MPI process.")
    params.addStringSubParam('queue', '#PBS -q QUEUE', "Which queue to submit this job to.")
    params.addStringSubParam('module', 'module load MODULE', 'moose-dev-gcc', "The module to load.")
    params.addStringSubParam('cli_args', 'CLI_ARGS', "Any extra command line arguments to tack on.")

    params.addRequiredParam('moose_application', "The full path to the application to run.")
    params.addRequiredParam('input_file', "The input file name.")

    return params
  getValidParams = staticmethod(getValidParams)

  def __init__(self, name, params):
    Job.__init__(self, name, params)

  def prepareJobScript(self):
    f = open(self.specs['template_script'], 'r')
    content = f.read()
    f.close()

    params = self.specs
    # Error check
    if params.isValid('mpi_procs') and params.isValid('total_mpi_procs'):
      print "ERROR: 'mpi_procs' and 'total_mpi_procs' are exclusive.  Only specify one!"
      sys.exit(1)

    # Do a few PBS job size calculations
    if params.isValid('mpi_procs'):
      params['mpi_procs_per_chunk'] = params['mpi_procs']
    elif params.isValid('total_mpi_procs'):
      params['mpi_procs_per_chunk'] = params['total_mpi_procs'] / params['chunks']  # Need some more error checking here
    else:
      print "ERROR: You must specify either 'mpi_procs' or 'total_mpi_procs'"
      sys.exit(1)
    if params.isValid('threads'):
      threads = int(params['threads'])
    else:
      threads = 1
    params['ncpus_per_chunk'] = str(int(params['mpi_procs_per_chunk']) * threads)

    f = open(os.path.split(params['template_script'])[1], 'w')

    # Do all of the replacements for the valid parameters
    for param in params.valid_keys():
      if param in params.substitute:
        params[param] = params.substitute[param].replace(param.upper(), params[param])
      content = content.replace('<' + param.upper() + '>', str(params[param]))

    # Make sure we strip out any string substitution parameters that were not supplied
    for param in params.substitute_keys():
      if not params.isValid(param):
        content = content.replace('<' + param.upper() + '>', '')

    f.write(content)
    f.close()

  def launch(self):
    # Finally launch the job
    subprocess.Popen('qsub ' + os.path.split(self.specs['template_script'])[1], shell=True)
