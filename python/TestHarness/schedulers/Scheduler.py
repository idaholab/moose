from time import sleep
from timeit import default_timer as clock
from collections import deque
from MooseObject import MooseObject
from Queue import Queue
from signal import SIGTERM
from util import *
import os, sys
from contribs import dag

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
            self.soft_limit = True
        else:
            self.job_slots = params['max_processes'] # hard limit
            self.soft_limit = False

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

        # Dictionary of dag testers
        self.shared_testers_dag = {}

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

    # Check for Race Conditions and fail all tests
    def checkRaceCondition(self, testers):
        # specifically check for race conditions, and fail the entire group immediately if any exist
        we_all_fail = False
        for tester in testers:
            if tester.checkRaceConditions(testers, self.options) is False:
                we_all_fail = True
                break
        if we_all_fail:
            for tester in testers:
                tester.setStatus('OUTFILE RACE CONDITION', tester.bucket_fail)
                self.harness.handleTestStatus(tester)
        return we_all_fail

    # Delete downstream tests from DAG associated with <tester>
    def deleteDownstreamTests(self, dag_object, tester):
        deleted_tester_set = set([])
        if dag_object.node_exists(tester.getTestName()):
            # Get all downstream tests
            for downstream_test_name in dag_object.all_downstreams(tester.getTestName()):
                dwn_tester = self.scheduled_name_to_object[downstream_test_name]
                # If this dwn_tester is initialized (that is, no one else has adjusted the status for this
                # test yet) and we are not ignoring prereqs, remove it.
                #
                # Note 1: This test may depend on other tests that were also skipped, so its possible to hit
                #         this method multiple times. Hence the check for an initialized status
                #
                # Note 2: Dry Run is here to override the tester setting this to PASS, because we still want
                #         to know when tests like this would normally be skipped due to a dependency issue
                #         (instead of printing out a green passing 'DRY RUN' status)
                if (dwn_tester.isInitialized() and not dwn_tester.skipPrereqs(self.options)) \
                   or self.options.dry_run:
                    # Set the status and assign the skipped bucket
                    dwn_tester.setStatus('skipped dependency', dwn_tester.bucket_skip)

                    # Add this deleted tester to a set
                    deleted_tester_set.add(dwn_tester)

                    # Ask a status thread to print our results
                    self.statusGo(downstream_test_name)

                # Like Note 1 above, the same applies here. We need to _attempt_ to delete the node, but not
                # tell the harness multiple times that this test was skipped. This 'downsteam' node just
                # happens to exist in mulitple places in the graph. Exp:
                # A     B
                #  \   /
                #    C  <--- Downstream 'C' node mentioned in both A and B.
                if not dwn_tester.skipPrereqs(self.options):
                    dag_object.delete_node_if_exists(downstream_test_name)

        # Return the set of tester objects we deleted
        return deleted_tester_set

    # Loop through the testers and add them to the runner pool for execution
    def schedule(self, testers):
        # We didn't recieve anything... pesky TestHarness
        if testers == []:
            return

        # Immediately test for race conditions and return if condition exists
        if self.checkRaceCondition(testers):
            return

        # Local storage of skipped tester objects
        skipped_or_failed_tests = set([])

        # Local storage of downstream deleted tester objects
        downstream_deleted = set([])

        # Local DAG
        tester_dag = dag.DAG()

        # Create the DAG, skipped_testers, and just about anything we can during this iteration loop
        for tester in testers:
            with self.thread_lock:
                # Create tester name to object map
                self.scheduled_name_to_object[tester.getTestName()] = tester

            # Create a node for this tester if not already exists (prereq iteration creates nodes)
            if not tester_dag.node_exists(tester.getTestName()):
                tester_dag.add_node(tester.getTestName())

            # Loop through prereqs and add leaves/nodes where necessary
            for prereq in tester.getPrereqs():

                # Discover unknown prereqs
                if prereq not in [x.getTestName() for x in testers]:
                    tester.setStatus('unknown dependency', tester.bucket_fail)
                    skipped_or_failed_tests.add(tester)
                    continue

                if not tester_dag.node_exists(prereq):
                    # Create a node for this tester
                    tester_dag.add_node(prereq)

                # Once you add an edge to a node, the DAG arranges the nodes accordingly
                try:
                    tester_dag.add_edge(prereq, tester.getTestName())

                # Handle Cyclic errors (tester status has already be applied)
                except dag.DAGValidationError:
                    pass

            # Discover skipped tests
            if not tester.getRunnable(self.options):
                skipped_or_failed_tests.add(tester)

        # Discover skipped tests and remove downstream tests that require this skipped test
        for tester in skipped_or_failed_tests:
            # Delete and return downstream nodes (prints 'skipped dependency' tests to the screen)
            downstream_deleted = self.deleteDownstreamTests(tester_dag, tester)

            # Delete this test
            tester_dag.delete_node_if_exists(tester.getTestName())

            # Inform the user it was skipped by adding it to the queue
            self.statusGo(tester.getTestName())

        # Get top level jobs we can launch
        ind_jobs = tester_dag.ind_nodes()

        # Maybe all jobs in this group got skipped?
        if ind_jobs == []:
            return

        # Add our local DAG to the global shared dag
        #with self.thread_lock:
        self.shared_testers_dag[testers[-1].getTestDir()] = tester_dag

        # Set a pending status on these jobs _before_ launching them
        for job in ind_jobs:
            self.scheduled_name_to_object[job].setStatus('QUEUED', self.scheduled_name_to_object[job].bucket_pending)

        # Launch the jobs
        for job in ind_jobs:
            self.runnerGo(job)

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

    # TODO: Possibly look into semaphore functionality for slot allocation
    def checkAvailableSlots(self, tester):
        # See if we can fit this job into our busy schedule
        if self.workers_in_use + tester.getProcs(self.options) <= self.job_slots:
            return True

        # Check for insufficient slots -soft limit
        # TODO: Create a unit test for this case
        elif tester.getProcs(self.options) > self.job_slots and self.soft_limit:
            tester.specs.addParam('caveats', ['OVERSIZED'], "")
            return True

        # Check for insufficient slots -hard limit (skip this job)
        # TODO: Create a unit test for this case
        elif tester.getProcs(self.options) > self.job_slots and not self.soft_limit:
            tester.setStatus('insufficient slots', tester.bucket_skip)

    # Status processing (hand a tester's status back to the TestHarness)
    def statusRunner(self, queue):
        tester_name = queue.get()
        tester = self.scheduled_name_to_object[tester_name]

        # Test is Finished (failed, passed, skipped, silent, etc)
        if tester.isFinished():
            # And the running caveat
            if tester_name in self.reported_jobs and tester.didPass():
                tester.specs.addParam('caveats', ['FINISHED'], "")

            # Print finalized results
            self.harness.handleTestStatus(tester)

            # Remove this finished tester from the DAG and active job set, get the next node, and assign it to
            # a runner thread
            with self.thread_lock:
                tester_dag = self.shared_testers_dag[tester.getTestDir()]

                # Handle dependencies that will fail if this test did not successfully finish but only if we care
                # about prereqs
                if not tester.didPass():
                    self.deleteDownstreamTests(tester_dag, tester)

                # Get the next list of runnable nodes (jobs)
                tester_dag.delete_node(tester_name)
                next_jobs = tester_dag.ind_nodes()

                # Loop through and determine if we need to assign a thread to this new job
                new_tmp_jobs = []
                for job in next_jobs:
                    tmp_tester = self.scheduled_name_to_object[job]
                    if job not in self.active_jobs and tmp_tester.isInitialized():
                        tmp_tester.setStatus('QUEUED', tmp_tester.bucket_pending)
                        new_tmp_jobs.append(job)

            # Add new jobs outside thread lock
            for job in new_tmp_jobs:
                self.runnerGo(job)

        # Test is Pending and could not run for some reason or another
        else:
            # The runner when performing checkRunnableBase, didn't fail or skip the test. But it couldn't
            # run for other reasons (not enough slots, etc). So place it back in the runner queue
            if tester_name not in self.active_jobs:
                self.runnerGo(tester_name)

            # We should spin here for just a bit; A runner and status thread will rapidly hand off a
            # job to each other for tests that can't run yet due to not enough slots being available
            #sleep(0.01)

    # Runner processing (call derived run method using a thread)
    def jobRunner(self, queue):
        # Get a job
        tester_name = queue.get()

        # Get the tester object based on tester_name
        tester = self.scheduled_name_to_object[tester_name]

        # Start the clock immediately (in case a derived scheduler plugin method fails to set this) or
        # in case we attempt to check the status on this job before hitting the run method (skipped tests)
        tester.setStartTime(clock())

        can_run = True
        with self.thread_lock:
            # Check if we have available slots
            if self.checkAvailableSlots(tester):
                # Add this job to our active list
                self.active_jobs.add(tester_name)

                # Record the amount of slots we're about to consume
                self.workers_in_use += tester.getProcs(self.options)

            # We can not run. And we need to perform the status queue operation outside of this thread lock
            else:
                can_run = False

        if can_run:
            # Call derived run method outside of the thread_lock (run is blocking)
            self.run(tester, self.thread_lock, self.options)

            # run should never set a pending status. If it did, something is wrong with the derived scheduler
            # TODO: create a unit test for this
            if tester.isPending():
                raise SchedulerError('Derived Scheduler can not return a pending status!')

            # Making modifications to the tester requires a thread lock
            with self.thread_lock:
                # Stop the clock (in case a derived scheduler plugin method fails to set this)
                if tester.getEndTime() is None:
                    tester.setEndTime(clock())

                # Recover our available slots
                self.workers_in_use = max(0, self.workers_in_use - tester.getProcs(self.options))

                # Remove this job from our active set
                self.active_jobs.remove(tester_name)

        # Add this job to the status queue for pending or finished work
        self.statusGo(tester_name)
