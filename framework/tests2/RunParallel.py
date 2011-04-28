from subprocess import *
from timeit import default_timer as clock

## This class provides an interface to run commands in parallel
#
# To use this class, call the .run() method with 
class RunParallel():

  ## Return this return code if the process must be killed because of timeout
  TIMEOUT = -999999

  def __init__(self, harness, max_processes=1):
    self.harness = harness
    self.max = max_processes
    self.jobs = [None] * self.max
    self.next_job_index = 0
    self.num_jobs = 0

  ## when command is finished running this calls callback(options, returncode, output)
  def run(self, test, command):
    output = 'Executing: ' + command + '\n'

    if self.num_jobs < self.max:
      self.spinwait()

    p = Popen([command],stdout=PIPE,stderr=STDOUT, close_fds=True, shell=True)
    #p.poll() p.terminate()
    output += p.communicate()[0]

    self.harness.testOutputAndFinish(test, p.returncode, output)

  ## Don't return until one of the running processes exits
  def spinwait(self):
    pass

  ## Check to make sure none of the running processes are out of time
  def checkTimes(self):
    pass

  ## Wait until all processes are done, then return
  def join(self):
    pass


## Static logging string for debugging
LOG = ''
LOG_ON = True
def log(msg):
  if LOG_ON:
    print msg
