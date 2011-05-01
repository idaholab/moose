from subprocess import *
from time import sleep
from timeit import default_timer as clock

from options import *

## This class provides an interface to run commands in parallel
#
# To use this class, call the .run() method with the command and the test
# options. When the test is finished running it will call harness.testOutputAndFinish
# to complete the test. Be sure to call join() to make sure all the tests are finished.
#
# If processes ever lock up when running jobs that produce a lot of output, the first place
# to looks is:
# http://stackoverflow.com/questions/5582933/need-to-avoid-subprocess-deadlock-without-communicate
class RunParallel():

  ## Return this return code if the process must be killed because of timeout
  TIMEOUT = -999999

  def __init__(self, harness, max_processes=1):
    ## The test harness to run callbacks on
    self.harness = harness

    ## List of currently running jobs as (Popen instance, command, test, time when expires) tuples
    # None means no job is running in this slot
    self.jobs = [None] * max_processes

  ## run the command asynchronously and call testharness.testOutputAndFinish when complete
  def run(self, test, command):

    # Wait for a job to finish if the jobs queue is full
    if self.jobs.count(None) == 0:
      self.spinwait()

    job_index = self.jobs.index(None) # find an empty slot
    log( 'Command %d started: %s' % (job_index, command) )
    p = Popen([command],stdout=PIPE,stderr=STDOUT, close_fds=True, shell=True)

    self.jobs[job_index] = (p, command, test, clock() + test[MAX_TIME])

    #p.wait()
    #self.returnToTestHarness(job_index)


  ## Return control the the test harness by finalizing the test output and calling the callback
  def returnToTestHarness(self, job_index):
    (p, command, test, time) = self.jobs[job_index]
    
    log( 'Command %d done:    %s' % (job_index, command) )
    if p.poll() == None: # process has not completed, it timed out
      p.terminate()
      self.harness.testOutputAndFinish(test, RunParallel.TIMEOUT, p.stdout.read())
    else:
      output = 'Running command: ' + command + '\n'
      output += p.communicate()[0]
      self.harness.testOutputAndFinish(test, p.returncode, output)

    self.jobs[job_index] = None

  ## Don't return until one of the running processes exits.
  #
  # When a process exits (or times out) call returnToTestHarness and return from
  # this function.
  def spinwait(self):
    test_completed = False
    while not test_completed:
      now = clock()
      job_index = 0
      for tuple in self.jobs:
        if tuple != None:
          (p, command, test, time) = tuple
          if p.poll() != None or now > time:
            test_completed = True
            self.returnToTestHarness(job_index)
        job_index += 1

      sleep(0.05) # sleep for 50ms


  ## Check to make sure none of the running processes are out of time
  #def checkTimes(self):
    #pass

  ## Wait until all processes are done, then return
  def join(self):
    while self.jobs.count(None) != len(self.jobs):
      self.spinwait()


## Static logging string for debugging
LOG = []
LOG_ON = False
def log(msg):
  LOG.append(msg)
  if LOG_ON:
    print msg
