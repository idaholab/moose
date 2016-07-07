from subprocess import *
from time import sleep
from timeit import default_timer as clock

from tempfile import TemporaryFile
#from Queue import Queue
from collections import deque
from Tester import Tester
from signal import SIGTERM
import platform

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

  def __init__(self, harness, max_processes=None, average_load=64.0):
    ## The test harness to run callbacks on
    self.harness = harness

    # Retrieve and store the TestHarness options for use in this object
    self.options = harness.getOptions()

    # For backwards compatibitliy the RunParallel class can be initialized
    # with no "max_processes" argument and it'll default to a soft limit.
    # If however a max_processes  is passed we'll treat it as a hard limit.
    # The difference is whether or not we allow single jobs to exceed
    # the number of slots.
    if max_processes == None:
      self.soft_limit = True
      self.job_slots = 1
    else:
      self.soft_limit = False
      self.job_slots = max_processes # hard limit

    # Current slots in use
    self.slots_in_use = 0

    ## List of currently running jobs as (Popen instance, command, test, time when expires, slots) tuples
    # None means no job is running in this slot
    self.jobs = [None] * self.job_slots

    # Requested average load level to stay below
    self.average_load = average_load

    # queue for jobs needing a prereq
    self.queue = deque()

    # queue for jobs that are always too big (can run at the end if we have soft limits)
    self.big_queue = deque()

    # Jobs that have been finished
    self.finished_jobs = set()

    # List of skipped jobs to resolve prereq issues for tests that never run
    self.skipped_jobs = set()

    # Jobs we are reporting as taking longer then 10% of MAX_TIME
    self.reported_jobs = set()

    # Reporting timer which resets when ever data is printed to the screen.
    self.reported_timer = clock()

  ## run the command asynchronously and call testharness.testOutputAndFinish when complete
  def run(self, tester, command, recurse=True, slot_check=True):
    # First see if any of the queued jobs can be run but only if recursion is allowed on this run
    if recurse:
      self.startReadyJobs(slot_check)

    # Get the number of slots that this job takes
    slots = tester.getProcs(self.options) * tester.getThreads(self.options)

    # Is this job always too big?
    if slot_check and slots > self.job_slots:
      if self.soft_limit:
        self.big_queue.append([tester, command, os.getcwd()])
      else:
        self.harness.handleTestResult(tester.specs, '', 'skipped (Insufficient slots)')
      return

    # Now make sure that this job doesn't have an unsatisfied prereq
    if tester.specs['prereq'] != None and len(set(tester.specs['prereq']) - self.finished_jobs) and self.options.pbs is None:
      self.queue.append([tester, command, os.getcwd()])
      return

    # Make sure we are complying with the requested load average
    self.satisfyLoad()

    # Wait for a job to finish if the jobs queue is full
    while self.jobs.count(None) == 0 or self.slots_in_use >= self.job_slots:
      self.spinwait()

    # Will this new job fit without exceeding the available job slots?
    if slot_check and self.slots_in_use + slots > self.job_slots:
      self.queue.append([tester, command, os.getcwd()])
      return

    # Pre-run preperation
    tester.prepare()

    job_index = self.jobs.index(None) # find an empty slot
    log( 'Command %d started: %s' % (job_index, command) )

    # It seems that using PIPE doesn't work very well when launching multiple jobs.
    # It deadlocks rather easy.  Instead we will use temporary files
    # to hold the output as it is produced
    try:
      if self.options.dry_run or not tester.shouldExecute():
        tmp_command = command
        command = "echo"

      f = TemporaryFile()

      # On Windows, there is an issue with path translation when the command is passed in
      # as a list.
      if platform.system() == "Windows":
        p = Popen(command,stdout=f,stderr=f,close_fds=False, shell=True, creationflags=CREATE_NEW_PROCESS_GROUP)
      else:
        p = Popen(command,stdout=f,stderr=f,close_fds=False, shell=True, preexec_fn=os.setsid)

      if self.options.dry_run or not tester.shouldExecute():
        command = tmp_command
    except:
      print "Error in launching a new task"
      raise

    self.jobs[job_index] = (p, command, tester, clock(), f, slots)
    self.slots_in_use = self.slots_in_use + slots

  def startReadyJobs(self, slot_check):
    queue_items = len(self.queue)
    for i in range(0, queue_items):
      (tester, command, dirpath) = self.queue.popleft()
      saved_dir = os.getcwd()
      sys.path.append(os.path.abspath(dirpath))
      os.chdir(dirpath)
      # We want to avoid "dual" recursion so pass a False flag here
      self.run(tester, command, recurse=False, slot_check=slot_check)
      os.chdir(saved_dir)
      sys.path.pop()

  ## Return control the the test harness by finalizing the test output and calling the callback
  def returnToTestHarness(self, job_index):
    (p, command, tester, time, f, slots) = self.jobs[job_index]

    log( 'Command %d done:    %s' % (job_index, command) )
    did_pass = True

    output = 'Working Directory: ' + tester.specs['test_dir'] + '\nRunning command: ' + command + '\n'
    output += self.readOutput(f)
    if p.poll() == None: # process has not completed, it timed out
      output += '\n' + "#"*80 + '\nProcess terminated by test harness. Max time exceeded (' + str(tester.specs['max_time']) + ' seconds)\n' + "#"*80 + '\n'
      f.close()
      if platform.system() == "Windows":
        p.terminate()
      else:
        pgid = os.getpgid(p.pid)
        os.killpg(pgid, SIGTERM)

      if not self.harness.testOutputAndFinish(tester, RunParallel.TIMEOUT, output, time, clock()):
        did_pass = False
    else:
      f.close()

      if tester in self.reported_jobs:
        tester.specs.addParam('caveats', ['FINISHED'], "")

      if not self.harness.testOutputAndFinish(tester, p.returncode, output, time, clock()):
        did_pass = False

    if did_pass:
      self.finished_jobs.add(tester.specs['test_name'])
    else:
      self.skipped_jobs.add(tester.specs['test_name'])

    self.jobs[job_index] = None
    self.slots_in_use = self.slots_in_use - slots

  ## Don't return until one of the running processes exits.
  #
  # When a process exits (or times out) call returnToTestHarness and return from
  # this function.
  def spinwait(self, time_to_wait=0.05):
    now = clock()
    job_index = 0
    slot_freed = False
    for tuple in self.jobs:
      if tuple != None:
        (p, command, tester, start_time, f, slots) = tuple
        if p.poll() != None or now > (start_time + float(tester.specs['max_time'])):
          # finish up as many jobs as possible, don't sleep until
          # we've cleared all of the finished jobs
          self.returnToTestHarness(job_index)

          # We just output to the screen so reset the test harness "activity" timer
          self.reported_timer = now

          slot_freed = True
          # We just reset the timer so no need to check if we've been waiting for awhile in
          # this iteration

        # Has the TestHarness done nothing for awhile
        elif now > (self.reported_timer + 10.0):
          # Has the current test been previously reported?
          if tester not in self.reported_jobs:
            if tester.specs.isValid('min_reported_time'):
              start_min_threshold = start_time + float(tester.specs['min_reported_time'])
            else:
              start_min_threshold = start_time + (0.1 * float(tester.specs['max_time']))

            threshold = max(start_min_threshold, (0.1 * float(tester.specs['max_time'])))

            if now >= threshold:
              self.harness.handleTestResult(tester.specs, '', 'RUNNING...', start_time, now, False)

              self.reported_jobs.add(tester)
              self.reported_timer = now

      job_index += 1

    if not slot_freed:
      sleep(time_to_wait)

  def satisfyLoad(self):
    # Get the current load average, or zero if it isn't available for some reason (such as being
    #   run on a non-posix operating system)
    loadAverage = 0.0
    try:
      loadAverage = os.getloadavg()[0]
    except AttributeError:
      pass      # getloadavg() not available in this implementation of os

    # We'll always run at least one job regardless of load or we'll starve!
    while self.jobs.count(None) < len(self.jobs) and loadAverage >= self.average_load:

#      print "DEBUG: Sleeping... ", len(self.jobs) - self.jobs.count(None), " jobs running (load average: ", os.getloadavg()[0], ")\n"
      self.spinwait(0.5) # If the load average is high we'll sleep longer here to let things clear out
#      print "DEBUG: Ready to run (load average: ", os.getloadavg()[0], ")\n"

  ## Wait until all processes are done, then return
  def join(self):
    while self.jobs.count(None) != len(self.jobs):
      self.spinwait()
      self.startReadyJobs(slot_check=True)

    # At this point there are no running jobs but there may still be jobs in queue
    # for three reasons:
    # 1) There are testers that require more slots than were available for this run.
    # 2) There is a tester that is waiting on a prereq that was skipped.
    # 3) There is an invalid or cyclic dependency in one or more test specifications

    # Handle the first case if the user has not explicitely provided a jobs argument
    # We'll allow larger jobs if the TestHarness is run with without any jobs argument
    if len(self.big_queue) and self.soft_limit:
      print "\nOversized Jobs:\n"

      # Dump the big jobs into the front of the queue
      self.queue.extendleft(self.big_queue)
      # Run the queue again without the slot check
      self.startReadyJobs(slot_check=False)
      while self.jobs.count(None) != len(self.jobs):
        self.spinwait()
        self.startReadyJobs(slot_check=False)

    # If we had a soft limit then we'll have run the oversized jobs but we still
    # have three cases (see note above) of jobs left to handle. We'll do that here
    if len(self.queue) != 0:
      keep_going = True
      while keep_going:
        keep_going = False
        queue_items = len(self.queue)
        for i in range(0, queue_items):
          (tester, command, dirpath) = self.queue.popleft()
          slots = tester.getProcs(self.options) * tester.getThreads(self.options)

          # If the user is running the script with no options, we'll just exceed the slots for
          # these remaining big jobs. Otherwise, we'll skip them
          if not self.soft_limit and slots > self.job_slots:
            self.harness.handleTestResult(tester.specs, '', 'skipped (Insufficient slots)')
            self.skipped_jobs.add(tester.specs['test_name'])
            keep_going = True
          # Do we have unsatisfied dependencies left?
          elif len(set(tester.specs['prereq']) & self.skipped_jobs):
            self.harness.handleTestResult(tester.specs, '', 'skipped (skipped dependency)')
            self.skipped_jobs.add(tester.specs['test_name'])
            keep_going = True
          # We need to keep trying in case there is a chain of unresolved dependencies
          # and we hit them out of order in this loop
          else:
            self.queue.append([tester, command, dirpath])

      # Anything left is a cyclic dependency
      if len(self.queue) != 0:
        print "\nCyclic or Invalid Dependency Detected!"
        for (tester, command, dirpath) in self.queue:
          print tester.specs['test_name']
        sys.exit(1)

  # This function reads output from the file (i.e. the test output)
  # but trims it down to the specified size.  It'll save the first two thirds
  # of the requested size and the last third trimming from the middle
  def readOutput(self, f, max_size=100000):
    first_part = int(max_size*(2.0/3.0))
    second_part = int(max_size*(1.0/3.0))
    output = ''

    f.seek(0)
    if self.harness.options.sep_files != True:
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
