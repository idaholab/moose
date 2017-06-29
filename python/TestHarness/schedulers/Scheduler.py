from time import sleep
from collections import deque
from MooseObject import MooseObject
from TesterData import TesterData
from Queue import Queue
from signal import SIGTERM
import os, sys, re
from contrib import dag

import util
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

    def __init__(self, harness, params):
        MooseObject.__init__(self, harness, params)

        ## The test harness to run callbacks on
        self.harness = harness

        # Retrieve and store the TestHarness options for use in this object
        self.options = harness.getOptions()

        # The Scheduler class can be initialized with no "max_processes" argument and it'll default
        # to a soft limit. If however a max_processes  is passed we'll treat it as a hard limit.
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

        # Thread Locking
        self.thread_lock = threading.RLock()

        # Workers in use
        self.workers_in_use = 0

        # A test name to object dictionary containing all tests that are scheduled for execution
        self.scheduled_tester_data = {}

        # A set of tests that are actively running
        self.active_jobs = set([])

        # A tester key to testers objects container {tester : testers}. Used when dealing with dependencies
        self.scheduled_groups = {}

        # A set holding reported jobs that have already informed the user of a 'RUNNING' status. Tests put in this set
        # will have the 'FINISHED' caveat applied to them
        self.reported_jobs = set([])

        # Runner Queue (we put jobs we want to run asyncronously in this)
        # runner_queue.put( tester_name )
        self.runner_queue = Queue()

        # Status queue (we put jobs we want to display a status for in this)
        # status_queue.put( tester_name )
        self.status_queue = Queue()

    ## Run the command asynchronously
    def run(self, tester):
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

    # return bool if we want to ignore prereqs requirements
    def skipPrereqs(self):
        if self.options.ignored_caveats:
            caveat_list = [x.lower() for x in self.options.ignored_caveats.split()]
            if 'all' in self.options.ignored_caveats or 'prereq' in self.options.ignored_caveats:
                return True
        return False

    # return bool for output file race conditions
    # NOTE: we return True for exceptions, but they are handled later (because we set a failure status)
    def checkRaceConditions(self, testers):
        d = util.DependencyResolver()
        name_to_object = {}

        for tester in testers:
            name_to_object[tester.getTestName()] = tester
            d.insertDependency(tester.getTestName(), tester.getPrereqs())
        try:
            # May fail, which will trigger an exception due to cyclic dependencies or key errors (unknown dependency)
            concurrent_tester_sets = d.getSortedValuesSets()
            for concurrent_testers in concurrent_tester_sets:
                output_files_in_dir = set()
                for tester in concurrent_testers:
                    if name_to_object[tester].getTestName() not in self.skipped_tests(testers):
                        output_files = name_to_object[tester].getOutputFiles()
                        duplicate_files = output_files_in_dir.intersection(output_files)
                        if len(duplicate_files):
                            return False
                        output_files_in_dir.update(output_files)
        except:
            # This is an unknown dependency or cyclic failure and will be handled elsewhere.
            pass
        return True

    # return a set of finished non-passing or will be skipped tests
    def skipped_tests(self, testers):
        skipped_failed = set([])
        for tester in testers:
            if (tester.isFinished() and not tester.didPass()) \
               or not tester.getRunnable(self.options) \
               or not tester.shouldExecute():
                skipped_failed.add(tester.getTestName())
        return skipped_failed

    # check for Race Conditions and fail the entire group
    def groupFailure(self, testers):
        we_all_fail = False
        for tester in testers:
            if self.checkRaceConditions(testers) is False:
                we_all_fail = True
                break

        if we_all_fail:
            for tester in testers:
                fake_dag = dag.DAG()
                tester_data = TesterData(tester, fake_dag, self.options)
                tester.setStatus('OUTFILE RACE CONDITION', tester.bucket_fail)
                self.harness.handleTestStatus(tester_data)

        return we_all_fail

    # Delete downstream tests from DAG associated with <tester>
    def deleteDownstreamTests(self, tester_data):
        tester = tester_data.getTester()
        dag_object = tester_data.getDAG()
        deleted_tester_set = set([])
        with self.thread_lock:
            if dag_object.node_exists(tester.getTestName()):
                # Get all downstream tests
                for downstream_test_name in dag_object.all_downstreams(tester.getTestName()):
                    dwn_tester = self.scheduled_tester_data[downstream_test_name].getTester()
                    if (dwn_tester.isInitialized() and not self.skipPrereqs()) \
                       or (self.options.dry_run and not dwn_tester.isSilent()):

                        # Set the status and assign test to the skipped bucket
                        dwn_tester.setStatus('skipped dependency', dwn_tester.bucket_skip)

                        # Add this deleted tester to a set
                        deleted_tester_set.add(dwn_tester)

                        # Ask a status thread to print our results
                        self.assignStatus(downstream_test_name)

                    if not self.skipPrereqs():
                        dag_object.delete_node_if_exists(downstream_test_name)

        # Return the set of tester objects we deleted
        return deleted_tester_set

    # Loop through the testers and add them to the runner pool for execution
    def schedule(self, testers):
        # Check for race conditions and cyclic dependencies and return if these issues exist
        if self.groupFailure(testers):
            return

        # Local storage of skipped tester objects
        skipped_or_failed_tests = set([])

        # Local DAG outside the tester loop (one DAG for entire group)
        tester_dag = dag.DAG()

        # Create the DAG, skipped_testers, and just about anything we can during this first iteration loop
        for tester in testers:

            # Local tester_data object to contain individual tester. The TesterData object is responsible
            # for handling the subprocess object and the output file handler.
            tester_data = TesterData(tester, tester_dag, self.options)

            with self.thread_lock:
                # Create tester_data map for each tester. This will contain and allow access to the the tester,
                # and the associated DAG for the entire tester group.
                self.scheduled_tester_data[tester.getTestName()] = tester_data

            # Create a node for this tester if not already exists (prereq iteration creates nodes)
            if not tester_dag.node_exists(tester.getTestName()):
                tester_dag.add_node(tester.getTestName())

            # Loop through prereqs and add leaves/nodes where necessary
            for prereq in tester.getPrereqs():

                # Discover unknown prereqs
                if prereq not in [x.getTestName() for x in testers]:
                    tester.setStatus('unknown dependency', tester.bucket_fail)
                    skipped_or_failed_tests.add(tester_data)
                    continue

                if not tester_dag.node_exists(prereq):
                    # Create a node for this tester
                    tester_dag.add_node(prereq)

                # Once you add an edge to a node, the DAG arranges the nodes accordingly
                try:
                    tester_dag.add_edge(prereq, tester.getTestName())

                # Handle Cyclic errors
                except dag.DAGValidationError:
                    tester.setStatus('Cyclic or Invalid Dependency Detected!', tester.bucket_fail)
                    skipped_or_failed_tests.add(tester_data)

                    # Pass so we allow the status runner to print the results serially
                    pass

            # Discover skipped tests
            if not tester.getRunnable(self.options):
                skipped_or_failed_tests.add(tester_data)

        # Discover skipped tests and remove downstream tests that require this skipped test. Any
        # discovered downstream nodes will have 'skipped dependency' set as their status.
        for skipped_tester_data in skipped_or_failed_tests:
            tmp_tester = skipped_tester_data.getTester()

            # Delete downstream nodes
            self.deleteDownstreamTests(skipped_tester_data)

            # Delete this node
            tester_dag.delete_node_if_exists(tmp_tester.getTestName())

            # Inform the user this node was skipped by adding it to the status queue
            self.assignStatus(tmp_tester.getTestName())

        # Is there anything to do? All tests were skipped?
        if tester_dag.size() == 0:
            return

        # Get top level nodes we can launch now
        ind_jobs = tester_dag.ind_nodes()

        # Set the status on all the nodes (jobs) this method is about to launch as QUEUED before we actually
        # launch them so as to prevent a very fast finishing job from asking for the next available job(s) this
        # method is about to launch (in other words; the status runner launches jobs that are set as INITIALIZED
        # and not already QUEUED while this method makes no comparison and 'just goes')
        for job in ind_jobs:
            tester = self.scheduled_tester_data[job].getTester()
            tester.setStatus('QUEUED', tester.bucket_pending)

        # With the job properly status'd above... launch them all asynchronously
        for job in ind_jobs:
            self.assignRunner(job)

    # Launch jobs stored in our queues. Method is blocked until both queue(s) and active jobs are empty
    def waitFinish(self):
        while len(self.active_jobs) != 0 or not self.runner_queue.empty() or not self.status_queue.empty():
            # sleep for just a tick or two
            sleep(0.5)

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

    # Create thread timers for various timing tasks (timeouts, long running)
    def handleTimers(self, start_timers=None, stop_timers=None):
        # Start some timers
        if start_timers:
            tester = start_timers.getTester()

            # Long running timer (RUNNING...)
            long_running_timer = threading.Timer(float(tester.getMinReportTime()),
                                                 self.doLongRunningJobs,
                                                 (tester,))
            long_running_timer.start()

            # Failed TIMEOUT timer
            timeout_timer = threading.Timer(float(tester.getMaxTime()),
                                          self.doTimeoutJobs,
                                          (self.scheduled_tester_data[tester.getTestName()],))
            timeout_timer.start()

            return (long_running_timer, timeout_timer)

        # Stop any timers we started
        elif stop_timers:
            for timer in stop_timers:
                timer.cancel()

    # Handle long running jobs
    def doLongRunningJobs(self, tester):
        tester.specs.addParam('caveats', ['FINISHED'], "")
        tester.setStatus('RUNNING...', tester.bucket_pending)
        self.assignStatus(tester.getTestName())

    # Handle jobs that are timing out
    def doTimeoutJobs(self, tester_data):
        tester = tester_data.getTester()
        tester.setStatus('TIMEOUT', tester.bucket_fail)
        tester_data.killProcess()

    def satisfyLoad(self):
        loadAverage = 0.0
        try:
            loadAverage = os.getloadavg()[0]
        except AttributeError:
            pass      # getloadavg() not available in this implementation of os
        # We'll always run at least one job regardless of load or we'll starve!
        while self.workers_in_use > 1 and loadAverage >= self.average_load:
            sleep(0.5)
            self.satisfyLoad()

    # TODO: Possibly look into semaphore functionality for slot allocation
    def checkAvailableSlots(self, tester):
        # Make sure we are complying with the requested load average
        self.satisfyLoad()

        with self.thread_lock:
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

    # Get next available set of jobs
    def getNextJobGroup(self, tester_data):
        tester = tester_data.getTester()
        tester_dag = tester_data.getDAG()
        next_group = []


        # Handle dependencies that will fail if this test did not successfully finish but only if we care
        # about prereqs
        if not tester.didPass():
            self.deleteDownstreamTests(self.scheduled_tester_data[tester.getTestName()])

        with self.thread_lock:
            # Get the next list of runnable nodes (jobs)
            tester_dag.delete_node(tester.getTestName())
            next_jobs = tester_dag.ind_nodes()
            # Loop through and determine if we need to assign a thread to this new job

            for next_job in next_jobs:
                tmp_tester = self.scheduled_tester_data[next_job].getTester()
                if next_job not in self.active_jobs and tmp_tester.isInitialized():
                    tmp_tester.setStatus('QUEUED', tmp_tester.bucket_pending)
                    next_group.append(next_job)

        return next_group

    # put a job in the runner queue and assign a runner thread to do some work
    def assignRunner(self, job):
        self.runner_queue.put(job)
        return self.runner_pool.apply_async(self.jobRunner, (self.runner_queue,))

    # put a job in the status queue and assign a status thread to do some work
    def assignStatus(self, job):
        self.status_queue.put(job)
        return self.status_pool.apply_async(self.statusRunner, (self.status_queue,))

    # Status processing (hand a tester's status back to the TestHarness to have some status printed to the screen)
    def statusRunner(self, queue):
        job = queue.get()
        self.harness.handleTestStatus(self.scheduled_tester_data[job])

    # Runner processing (call derived run method using a thread)
    def jobRunner(self, queue):
        job = queue.get()
        tester_dag = self.scheduled_tester_data[job].getDAG()
        tester = self.scheduled_tester_data[job].getTester()

        # Check if we have available slots
        if self.checkAvailableSlots(tester):
            can_run = True
            with self.thread_lock:
                # Add this job to our active list
                self.active_jobs.add(job)

                # Record the amount of slots we're about to consume
                self.workers_in_use += tester.getProcs(self.options)

        # We can not run just yet (slots full)
        else:
            sleep(0.03)
            self.assignRunner(job)
            return

        # Get a timer tuple of started thread timers
        my_timers = self.handleTimers(start_timers=self.scheduled_tester_data[job])

        # Call derived run method outside of the thread_lock (run is blocking)
        self.run(self.scheduled_tester_data[job])

        # The test finished, so stop any timers we received when starting them
        self.handleTimers(stop_timers=my_timers)

        # run should never set a pending status. If it did, something is wrong with the derived scheduler
        # TODO: create a unit test for this
        if tester.isPending():
            raise SchedulerError('Derived Scheduler can not return a pending status!')

        # Add this job to the status queue for finished work to be printed to the screen
        self.assignStatus(job)

        # This job is finished, prepare to launch other avialable jobs out of the DAG
        with self.thread_lock:
            # Recover our available slots
            self.workers_in_use = max(0, self.workers_in_use - tester.getProcs(self.options))

            # Remove this job from our active set
            self.active_jobs.remove(job)

        # Get the next group of jobs we can run
        next_job_group = self.getNextJobGroup(self.scheduled_tester_data[job])

        # Launch these next set of jobs
        for next_job in next_job_group:
            self.assignRunner(next_job)
