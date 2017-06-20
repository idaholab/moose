from time import sleep
from timeit import default_timer as clock
from collections import deque
from MooseObject import MooseObject
from Queue import Queue
from signal import SIGTERM
from util import *
import os, sys

from multiprocessing.pool import ThreadPool
import threading # for thread locking

class Scheduler(MooseObject):
    """
    Base class for handling how jobs are launched. To use this class, call .schedule()
    and supply a list of testers to schedule.

    Syntax:
       .schedule([list of tester objects])

    A list of testers will be added to a queue and begin calling the derived run method
    immediately. You can continue to add more testers to the queue in this fashion.

    Once you schedule all the testers you wish to test, call .waitFinish() to wait until
    all tests have finished.

    As tests finish individually, the scheduler will call back handleTestStatus(tester)
    """

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

        # For backwards compatibitliy the Scheduler class can be initialized
        # with no "max_processes" argument and it'll default to a soft limit.
        # If however a max_processes  is passed we'll treat it as a hard limit.
        # The difference is whether or not we allow single jobs to exceed
        # the number of slots.
        if params['max_processes'] == None:
            self.job_slots = 1
        else:
            self.job_slots = params['max_processes'] # hard limit

        # Requested average load level to stay below
        self.average_load = params['average_load']

        # Initialize runner pool based on available slots
        self.runner_pool = ThreadPool(processes=self.job_slots)

        # Initialize status pool to only use 1 process (to prevent status messages from getting clobbered)
        self.status_pool = ThreadPool(processes=1)

        # Set the time threshold when to report long running jobs
        self.default_reported_time = 10.0

        # Thread Locking
        self.thread_lock = threading.RLock()

        # Initialize Scheduler queue objects
        self.clearAndInitializeJobs()

    ## Clear and Initialize the scheduler queue
    def clearAndInitializeJobs(self):
        # Workers in use
        self.workers_in_use = 0

        # A test name to object dictionary containing all tests that are scheduled for execution
        self.scheduled_name_to_object = {}

        # A set of tests that are actively running
        self.active_jobs = set([])

        # A tester key to testers objects container {tester : testers}. Used when dealing with dependencies
        self.scheduled_groups = {}

        # A set holding reported jobs that have already informed the user of a 'RUNNING' status. Tests put in this set
        # will have the 'FINISHED' caveat applied to them
        self.reported_jobs = set([])

        # Runner Queue (we put jobs we want to run asyncronously in this)
        # runner_queue.put( (tester) )
        self.runner_queue = Queue()

        # Status queue (we put jobs we want to display a status for in this)
        # status_queue.put( (tester, clock(), subprocess_object) )
        self.status_queue = Queue()

    ## Run the command asynchronously
    def run(self, tester, thread_lock):
        return

    ## process test results
    def returnToTestHarness(self, tester, thread_lock):
        return

    # Return post run command from derived classes
    def postCommand(self):
        return

    # Allow derived schedulers to initiate findAndRunTests again
    def goAgain(self):
        return

    ## Returns a list of tests to run
    def getTests(self):
        return

    # Notify derived schedulers of status change
    def notifySchedulers(self, tester):
        return

    # Return boolean on output ready to be read
    def checkOutputReady(self, tester):
        return True

    # Call back to TestHarness to closeFiles in the event a derived scheduler wants to exit abnormally
    def cleanup(self, exit_code=0):
        self.harness.closeFiles(exit_code)

    # private methods to add a tester to a queue and assign a thread to it
    # TODO: refactor runnerGo and statusGo
    def runnerGo(self, tester_name):
        self.runner_queue.put(tester_name)
        return self.runner_pool.apply_async(self.jobRunner, (self.runner_queue,))

    # private methods to add a tester to a queue and assign a thread to it
    # TODO: refactor runnerGo and statusGo
    def statusGo(self, tester_name):
        self.status_queue.put(tester_name)
        return self.status_pool.apply_async(self.statusRunner, (self.status_queue,))

    # Loop through the testers and add them to the runner pool for execution
    def schedule(self, testers):

        # build a dictionary of testers so all threads have access to them
        for tester in testers:
            # { tester : testers } so that we know _this_ tester is related to these testers (dependency tracking)
            self.scheduled_groups[tester.getTestName()] = testers

            # { tester_name : tester_object } name to object map
            self.scheduled_name_to_object[tester.getTestName()] = tester

        # Add a test to the Runner Queue, and ask a thread to do some work (the thread pool has built in methods
        # which prevent oversubscribing)
        for tester in testers:
            results = self.runnerGo(tester.getTestName())
            # DEBUG this thread by using the get method. (Note: this is a blocking call)
            # print results.get()

    # Launch jobs stored in our queues. Method is blocked until both queue(s) and active jobs are empty
    def waitFinish(self):
        while len(self.active_jobs) != 0 or not self.runner_queue.empty() or not self.status_queue.empty():
            # Using a thread lock, check and do things every iteration
            with self.thread_lock:
                self.handleActiveTests()

            # sleep for just a tick or two
            sleep(0.1)

        ### Runner Pool
        # Wait for the runner pool to empty before waiting on the status pool
        # The runner pool and status pool can exhange jobs between each other but it is always the status pool
        # that will have items in the queue last (that last job with a finished status to be printed)

        # Close the runner_pool so jobs can no longer be scheduled for launching
        self.runner_pool.close()

        # Wait for the runner_pool to empty
        self.runner_pool.join()

        # Close the status_pool so jobs can no longer be scheduled for status printing
        self.status_pool.close()

        # Wait for the status_pool to empty
        self.status_pool.join()

    # Method to do things needed to actively running tests.
    # NOTE: We are in a thread locked state, so be quick
    def handleActiveTests(self):
        # loop through all the active tests and do something
        for tester_name in self.active_jobs:
            tester = self.scheduled_name_to_object[tester_name]

            # Handle long running tests
            self.doLongRunningTests(tester)

            # Handle timeouts
            self.doTimeoutTests(tester)

    def doLongRunningTests(self, tester):
        if tester.getTestName() not in self.reported_jobs:
            min_time = self.default_reported_time
            if tester.specs.isValid('min_reported_time'):
                min_time = float(tester.getMinReportTime())

            if clock() - tester.getStartTime() > min_time:
                self.reported_jobs.add(tester.getTestName())
                tester.setStatus('RUNNING...', tester.bucket_pending)
                self.harness.handleTestStatus(tester)

    def doTimeoutTests(self, tester):
        if tester.getTestName() not in self.reported_jobs:
            if clock() - tester.getStartTime() > float(tester.getMaxTime()):
                self.reported_jobs.add(tester.getTestName())
                tester.setStatus('TIMEOUT', tester.bucket_fail)
                self.harness.handleTestStatus(tester)

    # Status processing (hand a tester's status back to the TestHarness)
    def statusRunner(self, queue):
        tester_name = queue.get()
        tester = self.scheduled_name_to_object[tester_name]

        # check to see if this test is complete
        if tester.isFinished():
            # And the running caveat and
            if tester_name in self.reported_jobs and tester.didPass():
                tester.specs.addParam('caveats', ['FINISHED'], "")

            # Print finalized results
            # TODO: this prints TIMEOUT tests twice. Fix that.
            self.harness.handleTestStatus(tester)

        else:
            # place this job back in the runner/status queue because it was not ready to launch or is pending
            with self.thread_lock:
                # Make sure the job isn't already in the runner queue (job that is not skipped)
                if tester.getTestName() not in self.active_jobs:
                    self.runnerGo(tester_name)

                # Place this job back in the status queue (job is running and is not done)
                else:
                    self.statusGo(tester_name)

            # We should spin here for just a bit; A runner and status thread will rapidly hand off a
            # job to each other for tests that can't run yet due to a prereq not being ready.
            sleep(0.01)

    # Runner processing (blocking calls to the run method)
    def jobRunner(self, queue):
        # Get a job
        tester_name = queue.get()
        tester = self.scheduled_name_to_object[tester_name]

        # get associated testers
        testers =  self.scheduled_groups[tester_name]

        with self.thread_lock:
            # Check if we have enough slots available
            if self.workers_in_use + tester.getProcs(self.options) > self.job_slots:
                # We do not have enough slots available
                self.statusGo(tester_name)
                return
            else:
                # Tally the workers this job is about to use
                self.workers_in_use = self.workers_in_use + tester.getProcs(self.options)

                # Add this job to our active set
                self.active_jobs.add(tester_name)

        # Start the clock (in case a derived scheduler plugin method fails to set this)
        tester.setStartTime(clock())

        # Add this job to the status queue to handle an immediate skipped tests
        self.statusGo(tester_name)

        # Call derived run methods (blocking)
        self.run(tester,  testers, self.thread_lock, self.options)

        # Stop the clock (in case a derived scheduler plugin method fails to set this)
        if tester.getEndTime() is None:
            tester.setEndTime(clock())

        with self.thread_lock:
            # Remove this job from our active set
            self.active_jobs.remove(tester_name)

            # Recover our available slots
            self.workers_in_use = max(0, self.workers_in_use - tester.getProcs(self.options))
