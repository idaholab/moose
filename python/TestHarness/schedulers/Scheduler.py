from time import sleep
from timeit import default_timer as clock
from collections import deque
from MooseObject import MooseObject

import os, sys

## Base class for handling how jobs are launched
#
# To use this class, call the .run() method with the command and the test
# options. When the test is finished running it will call harness.testOutputAndFinish
# to complete the test. Be sure to call join() to make sure all the tests are finished.
class Scheduler(MooseObject):

    @staticmethod
    def validParams():
        params = MooseObject.validParams()
        params.addRequiredParam('average_load',          64.0, "Average load to allow")
        params.addRequiredParam('max_processes',         None, "Hard limit of maxium processes to use")

        return params

    def __init__(self, harness, params):
        MooseObject.__init__(self, harness, params)

        ## The test harness to run callbacks on
        self.harness = harness

        # Retrieve and store the TestHarness options for use in this object
        self.options = harness.getOptions()

        # For backwards compatibitliy the RunParallel class can be initialized
        # with no "max_processes" argument and it'll default to a soft limit.
        # If however a max_processes  is passed we'll treat it as a hard limit.
        # The difference is whether or not we allow single jobs to exceed
        # the number of slots.
        if params['max_processes'] == None:
            self.soft_limit = True
            self.job_slots = 1
        else:
            self.soft_limit = False
            self.job_slots = params['max_processes'] # hard limit

        # Current slots in use
        self.slots_in_use = 0

        ## List of currently running jobs as (Popen instance, command, test, time when expires, slots) tuples
        # None means no job is running in this slot
        self.jobs = [None] * self.job_slots

        # Requested average load level to stay below
        self.average_load = params['average_load']

        # queue for jobs needing a prereq
        self.queue = deque()

        # queue for jobs that are always too big (can run at the end if we have soft limits)
        self.big_queue = deque()

        # Jobs that have finished
        self.finished_jobs = set()

        # List of skipped jobs to resolve prereq issues for tests that never run
        self.skipped_jobs = set()

        # Jobs we are reporting as taking longer then 10% of MAX_TIME or that we waited on for output files
        self.reported_jobs = set()

        # Jobs that take too long to prepare their output files
        self.files_not_ready = set()

        # Default time to wait before reporting long running jobs
        self.default_reporting_time = 10.0 # Seconds (should be a float)

        # Reporting timer which resets when ever data is printed to the screen.
        self.reported_timer = clock()

    ## run the command asynchronously and call testharness.testOutputAndFinish when complete
    def run(self, tester, command, recurse=True, slot_check=True):
        return

    ## Return control to the test harness by finalizing the test output and calling the callback
    def returnToTestHarness(self, job_index):
        return

    ## Returns False if all pre-preqs have been satisfied
    def unsatisfiedPrereqs(self, tester):
        if tester.specs['prereq'] != [] and len(set(tester.specs['prereq']) - self.finished_jobs):
            if self.options.ignored_caveats is None:
                return True
            else:
                if self.options.ignored_caveats:
                    caveat_list = [x.lower() for x in self.options.ignored_caveats.split()]
                    if 'all' not in self.options.ignored_caveats and 'prereq' not in self.options.ignored_caveats:
                        return True
        return False

    ## Attempt to launch jobs that can be launched
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
                # Look for completed tests
                if p.poll() != None:
                    if not self.checkOutputReady(tester):
                        # Here we'll use the files_not_ready set to start a new timer
                        # for waiting for files so that we can do somthing else while we wait
                        if tester not in self.files_not_ready:
                            self.jobs[job_index] = (p, command, tester, clock(), f, slots)
                            self.files_not_ready.add(tester)
                        # We've been waiting for awhile, let's read what we've got
                        # and send it back to the tester.
                        elif now > (start_time + self.default_reporting_time):
                            # Report
                            self.returnToTestHarness(job_index)
                            self.reported_timer = now
                            slot_freed = True

                    # Output is ready
                    else:
                        # Report
                        self.returnToTestHarness(job_index)
                        self.reported_timer = now
                        slot_freed = True

                # Timeouts
                elif now > (start_time + float(tester.specs['max_time'])):
                    # Report
                    self.returnToTestHarness(job_index)
                    self.reported_timer = now
                    slot_freed = True

                # Has the TestHarness done nothing for awhile?
                elif now > (self.reported_timer + self.default_reporting_time):
                    # Has the current test been previously reported?
                    if tester not in self.reported_jobs:
                        seconds_to_report = self.default_reporting_time
                        if tester.specs.isValid('min_reported_time'):
                            seconds_to_report = float(tester.specs['min_reported_time'])

                        if now >= self.reported_timer + seconds_to_report:
                            tester.setStatus('RUNNING...', tester.bucket_pending)
                            self.harness.handleTestStatus(tester)
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

            # print "DEBUG: Sleeping... ", len(self.jobs) - self.jobs.count(None), " jobs running (load average: ", os.getloadavg()[0], ")\n"
            self.spinwait(0.5) # If the load average is high we'll sleep longer here to let things clear out
            # print "DEBUG: Ready to run (load average: ", os.getloadavg()[0], ")\n"

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
                        tester.setStatus('Insufficient slots', tester.bucket_skip)
                        self.harness.handleTestStatus(tester)
                        self.skipped_jobs.add(tester.specs['test_name'])
                        keep_going = True
                    # Do we have unsatisfied dependencies left?
                    elif len(set(tester.specs['prereq']) & self.skipped_jobs):
                        tester.setStatus('skipped dependency', tester.bucket_skip)
                        self.harness.handleTestStatus(tester)
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
