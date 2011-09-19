from subprocess import *
from time import sleep
from timeit import default_timer as clock

from options import *
from tempfile import TemporaryFile
from Queue import Queue
import os, sys

## This class provides an interface to run commands in parallel
#
# To use this class, call the .run() method with the command and the test
# options. When the test is finished running it will call harness.testOutputAndFinish
# to complete the test. Be sure to call join() to make sure all the tests are finished.
#
class RunParallel:

  ## Return this return code if the process must be killed because of timeout
  TIMEOUT = -999999

  def __init__(self, harness, max_processes=1):
    ## The test harness to run callbacks on
    self.harness = harness

    ## List of currently running jobs as (Popen instance, command, test, time when expires) tuples
    # None means no job is running in this slot
    self.jobs = [None] * max_processes

    # Queue for jobs needing a prereq
    self.queue = Queue()

    # Jobs that have been finished
    self.finished_jobs = set()

  ## run the command asynchronously and call testharness.testOutputAndFinish when complete
  def run(self, test, command):

    # Make sure the job doesn't have an unsatisfied prereq
    if test[PREREQ] != None and not test[PREREQ] in self.finished_jobs:
      self.queue.put([test, command, os.getcwd()])
      return

    # Wait for a job to finish if the jobs queue is full
    # Note: This needs to be a while statement, not an if statement.
    #   "spinwait" calls "returnToTestHarness" which calls "startReadyJobs" which
    #   again calls "run" (this function).  That last call may start a previously queued
    #   job which will preempt the open slot in the jobs array.  Upon unwinding the stack
    #   another check of the jobs array is needed before launching the current test
    while self.jobs.count(None) == 0:
      self.spinwait()

    job_index = self.jobs.index(None) # find an empty slot
    log( 'Command %d started: %s' % (job_index, command) )

    # It seems that using PIPE doesn't work very well when launching multiple jobs.
    # It deadlocks rather easy.  Instead we will use temporary files
    # to hold the output as it is produced
    f = TemporaryFile()
    p = Popen([command],stdout=f,stderr=STDOUT,close_fds=False, shell=True)

    self.jobs[job_index] = (p, command, test, clock() + test[MAX_TIME], f)

  def startReadyJobs(self):
    queue_items = self.queue.qsize()
    for i in range(0, queue_items):
      (test, command, dirpath) = self.queue.get()
      saved_dir = os.getcwd()
      sys.path.append(os.path.abspath(dirpath))
      os.chdir(dirpath)
      self.run(test, command)
      os.chdir(saved_dir)
      sys.path.pop()

  ## Return control the the test harness by finalizing the test output and calling the callback
  def returnToTestHarness(self, job_index):
    (p, command, test, time, f) = self.jobs[job_index]

    self.finished_jobs.add(test[TEST_NAME])
    log( 'Command %d done:    %s' % (job_index, command) )
    if p.poll() == None: # process has not completed, it timed out
      f.seek(0)
      data = f.read()
      data += '\n###########################################################################\n' + \
          'Process terminated by test harness' + \
          '\n###########################################################################\n'
      p.terminate()
      self.harness.testOutputAndFinish(test, RunParallel.TIMEOUT, data)
    else:
      output = 'Running command: ' + command + '\n'
      f.seek(0)
      output += f.read()
      f.close()
      self.harness.testOutputAndFinish(test, p.returncode, output)

    self.jobs[job_index] = None
    self.startReadyJobs()

  ## Don't return until one of the running processes exits.
  #
  # When a process exits (or times out) call returnToTestHarness and return from
  # this function.
  def spinwait(self):
    now = clock()
    job_index = 0
    for tuple in self.jobs:
      if tuple != None:
        (p, command, test, time, f) = tuple
        if p.poll() != None or now > time:
          self.returnToTestHarness(job_index)
          break
      job_index += 1

    sleep(0.05) # sleep for 50ms

  ## Wait until all processes are done, then return
  def join(self):
    while self.jobs.count(None) != len(self.jobs):
      self.spinwait()


## Static logging string for debugging
LOG = []
LOG_ON = False
def log(msg):
  if LOG_ON:
    LOG.append(msg)
    print msg
