from subprocess import *
from time import sleep
from timeit import default_timer as clock

from options import *
from tempfile import TemporaryFile
#from Queue import Queue
from collections import deque
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

  def __init__(self, harness, max_processes=1, average_load=64.0):
    ## The test harness to run callbacks on
    self.harness = harness

    ## List of currently running jobs as (Popen instance, command, test, time when expires) tuples
    # None means no job is running in this slot
    self.jobs = [None] * max_processes

    # Requested average load level to stay below
    self.average_load = average_load

    # queue for jobs needing a prereq
    self.queue = deque()

    # Jobs that have been finished
    self.finished_jobs = set()

    # List of skipped jobs to resolve prereq issues for tests that never run
    self.skipped_jobs = set()

  ## run the command asynchronously and call testharness.testOutputAndFinish when complete
  def run(self, test, command, recurse=True):
    # First see if any of the queued jobs can be run but only if recursion is allowed on this run
    if recurse:
      self.startReadyJobs()

    # Now make sure that this job doesn't have an unsatisfied prereq
    if test[PREREQ] != None and not test[PREREQ] in self.finished_jobs:
      self.queue.append([test, command, os.getcwd()])
      return

    # Make sure we are complying with the requested load average
    self.satisfyLoad()

    # Wait for a job to finish if the jobs queue is full
    while self.jobs.count(None) == 0:
      self.spinwait()

    job_index = self.jobs.index(None) # find an empty slot
    log( 'Command %d started: %s' % (job_index, command) )

    # It seems that using PIPE doesn't work very well when launching multiple jobs.
    # It deadlocks rather easy.  Instead we will use temporary files
    # to hold the output as it is produced
    try:
      f = TemporaryFile()
      p = Popen([command],stdout=f,stderr=STDOUT,close_fds=False, shell=True)
    except:
      print "Error in launching a new task"
      raise

    self.jobs[job_index] = (p, command, test, clock() + test[MAX_TIME], f)

  def startReadyJobs(self):
    queue_items = len(self.queue)
    for i in range(0, queue_items):
      (test, command, dirpath) = self.queue.popleft()
      saved_dir = os.getcwd()
      sys.path.append(os.path.abspath(dirpath))
      os.chdir(dirpath)
      # We want to avoid "dual" recursion so pass a False flag here
      self.run(test, command, False)
      os.chdir(saved_dir)
      sys.path.pop()

  ## Return control the the test harness by finalizing the test output and calling the callback
  def returnToTestHarness(self, job_index):
    (p, command, test, time, f) = self.jobs[job_index]

    log( 'Command %d done:    %s' % (job_index, command) )
    did_pass = True

    if p.poll() == None: # process has not completed, it timed out
      output = self.readOutput(f)
      output += '\n' + "#"*80 + '\nProcess terminated by test harness\n' + "#"*80 + '\n'
      f.close()
      p.terminate()

      if not self.harness.testOutputAndFinish(test, RunParallel.TIMEOUT, output, time, time, clock()):
        did_pass = False
    else:
      output = 'Running command: ' + command + '\n'
      output += self.readOutput(f)
      f.close()
      if not self.harness.testOutputAndFinish(test, p.returncode, output, time, clock()):
        did_pass = False

    if did_pass:
      self.finished_jobs.add(test[TEST_NAME])
    else:
      self.skipped_jobs.add(test[TEST_NAME])

    self.jobs[job_index] = None

  ## Don't return until one of the running processes exits.
  #
  # When a process exits (or times out) call returnToTestHarness and return from
  # this function.
  def spinwait(self, time_to_wait=0.05):
    now = clock()
    job_index = 0
    for tuple in self.jobs:
      if tuple != None:
        (p, command, test, time, f) = tuple
        if p.poll() != None or now > time:
          self.returnToTestHarness(job_index)
          break
      job_index += 1

    sleep(time_to_wait)

  def satisfyLoad(self):
    # We'll always run at least one job regardless of load or we'll starve!
    while self.jobs.count(None) < len(self.jobs) and os.getloadavg()[0] >= self.average_load:
#      print "DEBUG: Sleeping... ", len(self.jobs) - self.jobs.count(None), " jobs running (load average: ", os.getloadavg()[0], ")\n"
      self.spinwait(0.5) # If the load average is high we'll sleep longer here to let things clear out
#      print "DEBUG: Ready to run (load average: ", os.getloadavg()[0], ")\n"

  ## Wait until all processes are done, then return
  def join(self):
    while self.jobs.count(None) != len(self.jobs):
      self.spinwait()
      self.startReadyJobs()

    if len(self.queue) != 0:
      # See if there are any tests left in the queue simply because their dependencies where skipped
      keep_going = True
      while keep_going:
        keep_going = False
        queue_items = len(self.queue)
        for i in range(0, queue_items):
          (test, command, dirpath) = self.queue.popleft()
          if test[PREREQ] in self.skipped_jobs:
            self.harness.handleTestResult(test, '', 'skipped (skipped dependency)')
            self.skipped_jobs.add(test[TEST_NAME])
            keep_going = True
          else:
            self.queue.append([test, command, dirpath])
      # Anything left is a cyclic dependency
      if len(self.queue) != 0:
        print "Cyclic or Invalid Dependency Detected!"
        for (test, command, dirpath) in self.queue:
          print test[TEST_NAME]
        sys.exit(1)

  # This function reads output from the file (i.e. the test output)
  # but trims it down to the specified size.  It'll save the first two thirds
  # of the requested size and the last third trimming from the middle
  def readOutput(self, f, max_size=1048576):
    first_part = int(max_size*(2.0/3.0))
    second_part = int(max_size*(1.0/3.0))

    f.seek(0)
    output = f.read(first_part)     # Limit the output to 1MB
    if len(output) == first_part:   # This means we didn't read the whole file yet
      output += "\n" + "#"*80 + "\n\nOutput trimmed\n\n" + "#"*80 + "\n"
      f.seek(-second_part, 2)       # Skip the middle part of the file

      if (f.tell() <= first_part):  # Don't re-read some of what you've already read
        f.seek(first_part+1, 0)

    output += f.read()              # Now read the rest
    return output


  # Add a skipped job to the list
  def jobSkipped(self, name):
    self.skipped_jobs.add(name)

## Static logging string for debugging
LOG = []
LOG_ON = False
def log(msg):
  if LOG_ON:
    LOG.append(msg)
    print msg
