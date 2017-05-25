from subprocess import *
from time import sleep
from timeit import default_timer as clock
from tempfile import TemporaryFile
from collections import deque
from MooseObject import MooseObject

from signal import SIGTERM
import os, sys, platform

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

    ## Return this return code if the process must be killed because of timeout
    TIMEOUT = -999999

    def __init__(self, harness, params):
        MooseObject.__init__(self, harness, params)

        self.specs = params

        ## The test harness to run callbacks on
        self.harness = harness

        # Retrieve and store the TestHarness options for use in this object
        self.options = harness.getOptions()

        # Set max jobs to allow
        params['max_processes'] = self.options.jobs
        params['average_load'] = self.options.load

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

        # Requested average load level to stay below
        self.average_load = params['average_load']

        # Initialize Scheduler queue objects
        self.clearAndInitializeJobs()

    # Return post run command from derived classes
    def postCommand(self):
        return

    # Allow derived schedulers to initiate FindAunTests again
    def goAgain(self):
        return

    # Allow derived schedulers to skip tests
    def canLaunch(self, tester, checks, test_list):
        return tester.checkRunnableBase(self.options, checks, test_list)

    ## run the command asynchronously and call testharness.testOutputAndFinish when complete
    def run(self, tester, command, recurse=True, slot_check=True):
        return

    ## Return control to the test harness by finalizing the test output and calling the callback
    def returnToTestHarness(self, job_index):
        return

    ## Returns a list of tests to run
    def getTests(self):
        return

    # Notify schedulers of status change
    def notifySchedulers(self, tester):
        return

    # Call back to TestHarness closeFiles
    def cleanup(self, exit_code=0):
        self.harness.closeFiles(exit_code)

    # Notify TestHarness of status change
    def notifySchedulersBase(self, tester, add_to_table=False):
        # Notify derived schedulers of status change
        self.notifySchedulers(tester)

        # Notify the TestHarness (print status)
        self.harness.handleTestStatus(tester, add_to_table=add_to_table)

    ## Clear and Initialize the scheduler queue
    def clearAndInitializeJobs(self):
        # Current slots in use
        self.slots_in_use = 0

        ## List of currently running jobs as (Popen instance, command, test, time when expires, slots) tuples
        # None means no job is running in this slot
        self.jobs = [None] * self.job_slots

        # queue for jobs needing a prereq
        self.queue = deque()

        # queue for jobs that are always too big (can run at the end if we have soft limits)
        self.big_queue = deque()

        # Jobs that have finished
        self.finished_jobs = set()

        # Jobs that have launched
        self.launched_jobs = set()

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

    ## Return control to the test harness by finalizing the test output and calling the callback
    def returnToTestHarnessBase(self, job_index):
        (p, command, tester, time, f, slots) = self.jobs[job_index]

        log( 'Command %d done:    %s' % (job_index, command) )
        output = 'Working Directory: ' + tester.specs['test_dir'] + '\nRunning command: ' + command + '\n'

        if p.poll() == None: # process has not completed, it timed out
            output += '\n' + "#"*80 + '\nProcess terminated by test harness. Max time exceeded (' + str(tester.specs['max_time']) + ' seconds)\n' + "#"*80 + '\n'
            f.close()
            tester.setStatus('TIMEOUT', tester.bucket_fail)
            if platform.system() == "Windows":
                p.terminate()
            else:
                pgid = os.getpgid(p.pid)
                os.killpg(pgid, SIGTERM)

            self.harness.testOutputAndFinish(tester, Scheduler.TIMEOUT, output, time, clock())
        else:
            # Handle --dru-run
            if self.options.dry_run:
                tester.success_message = 'DRY RUN'
                output += '\n'.join(tester.processResultsCommand(tester.getMooseDir(), self.options))
                self.harness.testOutputAndFinish(tester, p.returncode, output, time, clock())
                if tester.didPass():
                    self.finished_jobs.add(tester.specs['test_name'])
                else:
                    self.skipped_jobs.add(tester.specs['test_name'])
            else:
                # Allow derived schedulers to perform postprocessing on results
                self.returnToTestHarness(job_index, output)

            # Close the output file
            f.close()

        # Clear this job slot
        self.jobs[job_index] = None
        self.slots_in_use = self.slots_in_use - slots

    ## Returns False if all pre-preqs have been satisfied
    def unsatisfiedPrereqs(self, tester):
        if tester.getPrereqs() != [] and len(set(tester.getPrereqs()) - self.finished_jobs):
            if self.options.ignored_caveats is None:
                return True
            elif self.options.ignored_caveats != 'all':
                caveat_list = [x.lower() for x in self.options.ignored_caveats.split()]
                if 'prereq' not in caveat_list:
                    return True
        return False

    ## Schedule job for running
    def schedule(self, tester, checks, test_list=None):
        should_run = self.canLaunch(tester, checks, test_list)

        # check for deprecated tuple
        if type(should_run) == type(()):
            (should_run, reason) = should_run
            if not should_run:
                reason = 'deprected checkRunnableBase #8037'
                tester.setStatus(reason, tester.bucket_skip)
                self.notifySchedulersBase(tester, add_to_table=True)
                self.jobSkipped(tester.getTestName())

        # This job is allowed to run
        if should_run:
            command = tester.getCommand(self.options)
            # This method spawns another process and allows this loop to continue looking for tests
            # RunParallel will call self.testOutputAndFinish when the test has completed running
            # This method will block when the maximum allowed parallel processes are running
            self.run(tester, command)

        # This job is skipped - notify the scheduler
        else:
            if not tester.isSilent(): # SILENT occurs when a user is using --re options
                if (self.options.report_skipped and tester.isSkipped()) \
                   or tester.isSkipped():
                    self.notifySchedulersBase(tester, add_to_table=True)
                elif tester.isDeleted() and self.options.extra_info:
                    self.notifySchedulersBase(tester, add_to_table=True)
            self.jobSkipped(tester.getTestName())

    ## Find an empty slot and launch the job
    def launchJob(self, tester, command, slot_check=True):
        # Make sure we are complying with the requested load average
        self.satisfyLoad()

        # Wait for a job to finish if the jobs queue is full
        while self.jobs.count(None) == 0 or self.slots_in_use >= self.job_slots:
            self.spinwait()

        # Get the number of slots that this job takes
        slots = tester.getProcs(self.options) * tester.getThreads(self.options)

        # Is this job always too big?
        if slot_check and slots > self.job_slots:
            if self.soft_limit:
                self.big_queue.append([tester, command, os.getcwd()])
            else:
                tester.setStatus('Insufficient slots', tester.bucket_skip)
                self.notifySchedulersBase(tester, add_to_table=True)
                self.skipped_jobs.add(tester.specs['test_name'])
            return

        # Will this new job fit without exceeding the available job slots?
        if slot_check and self.slots_in_use + slots > self.job_slots:
            self.queue.append([tester, command, os.getcwd()])
            return False

        job_index = self.jobs.index(None) # find an empty slot
        log( 'Command %d started: %s' % (job_index, command) )

        # Pre-run preperation
        # TODO: Does this belong here in the base class?
        tester.prepare(self.options)

        # Record the launched job
        self.launched_jobs.add(tester.specs['test_name'])

        # It seems that using PIPE doesn't work very well when launching multiple jobs.
        # It deadlocks rather easy.  Instead we will use temporary files
        # to hold the output as it is produced
        try:

            f = TemporaryFile()
            # On Windows, there is an issue with path translation when the command is passed in
            # as a list.
            if platform.system() == "Windows":
                p = Popen(command,stdout=f,stderr=f,close_fds=False, shell=True, creationflags=CREATE_NEW_PROCESS_GROUP)
            else:
                p = Popen(command,stdout=f,stderr=f,close_fds=False, shell=True, preexec_fn=os.setsid)

            self.jobs[job_index] = (p, command, tester, clock(), f, slots)
            self.slots_in_use = self.slots_in_use + slots

        except:
            print "Error in launching a new task"
            raise

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
                            self.returnToTestHarnessBase(job_index)
                            self.reported_timer = now
                            slot_freed = True

                    # Output is ready
                    else:
                        # Report
                        self.returnToTestHarnessBase(job_index)
                        self.reported_timer = now
                        slot_freed = True

                # Timeouts
                elif now > (start_time + float(tester.specs['max_time'])):
                    # Report
                    self.returnToTestHarnessBase(job_index)
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
                            self.notifySchedulersBase(tester)
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
                        self.notifySchedulersBase(tester, add_to_table=True)
                        self.skipped_jobs.add(tester.getTestName())
                        keep_going = True
                    # Do we have unsatisfied dependencies left?
                    elif len(set(tester.specs['prereq']) & self.skipped_jobs):
                        tester.setStatus('skipped dependency', tester.bucket_skip)
                        self.notifySchedulersBase(tester, add_to_table=True)
                        self.skipped_jobs.add(tester.getTestName())
                        keep_going = True
                    # We need to keep trying in case there is a chain of unresolved dependencies
                    # and we hit them out of order in this loop
                    else:
                        self.queue.append([tester, command, dirpath])

            # Anything left is a cyclic dependency
            if len(self.queue) != 0:
                print "\nCyclic or Invalid Dependency Detected!"
                for (tester, command, dirpath) in self.queue:
                    print tester.getTestName()
                self.harness.closeFiles(1)

    # Add a skipped job to the list
    def jobSkipped(self, name):
        self.skipped_jobs.add(name)

    def checkOutputReady(self, tester):
        # See if the output is available, it'll be available if we are just using the TemporaryFile mechanism
        # If we are redirecting output though, it might take a few moments for the file buffers to appear
        # after one of the process quits.
        if not tester.specs.isValid('redirect_output') or not tester.specs['redirect_output'] or tester.getProcs(self.options) == 1:
            return True

        for processor_id in xrange(tester.getProcs(self.options)):
            # Obtain path and append processor id to redirect_output filename
            file_path = os.path.join(tester.getTestName(), tester.name() + '.processor.{}'.format(processor_id))
            if not os.access(file_path, os.R_OK):
                return False
        return True

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

## Static logging string for debugging
LOG = []
LOG_ON = False
def log(msg):
    if LOG_ON:
        LOG.append(msg)
        print msg
