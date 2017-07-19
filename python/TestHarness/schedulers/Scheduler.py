from time import sleep
from collections import deque
from MooseObject import MooseObject
from TesterData import TesterData
from signal import SIGTERM
import os, sys, re, copy
from contrib import dag

import util
from multiprocessing.pool import ThreadPool

# Prototype parallel non-shared work
from multiprocessing import Process

import threading # for thread locking
import multiprocessing # for timeouts

class SchedulerError(Exception):
    pass

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
            self.available_slots = 1
            self.soft_limit = True
        else:
            self.available_slots = params['max_processes'] # hard limit
            self.soft_limit = False

        # Requested average load level to stay below
        self.average_load = params['average_load']

        # Initialize tester pool based on available slots
        self.tester_pool = ThreadPool(processes=self.available_slots)

        # Initialize status pool to only use 1 process (to prevent status messages from getting clobbered)
        self.status_pool = ThreadPool(processes=1)

        # Slot Lock when processing resource allocations
        self.slot_lock = threading.Lock()

        # DAG Lock when processing the DAG
        self.dag_lock = threading.Lock()

        # Workers in use
        self.slots_in_use = 0

        # Jobs waiting to finish
        self.job_queue_count = 0

        # Set containing our Tester Data containers. We use this in the event of a KeyboardInterrupt to
        # iterate over and kill any subprocesses
        self.tester_datas = set([])

    ## In the event of a KeyboardInterrupt, kill all Popen processes
    def killRemaining(self):
        # Close the status_pool so as to prevent further tester statuses from printing to the screen
        self.status_pool.close()

        # Loop through the known testers and attempt to kill their subprocess
        for tester_data in self.tester_datas:
            tester_data.killProcess()

    ## Run the command asynchronously
    def run(self, job_container):
        return

    # return bool if we want to ignore prereq requirements
    def skipPrereqs(self):
        if self.options.ignored_caveats:
            caveat_list = [x.lower() for x in self.options.ignored_caveats.split()]
            if 'all' in self.options.ignored_caveats or 'prereq' in self.options.ignored_caveats:
                return True
        return False

    # return a set of finished non-passing or will be skipped tests
    def skippedTests(self, testers):
        skipped_failed = set([])
        for tester in testers:
            if (tester.isFinished() and not tester.didPass()) \
               or not tester.getRunnable(self.options):
                skipped_failed.add(tester)
        return skipped_failed

    # Delete and return downstream nodes based on tester status and TestHarness options
    def processDownstreamTests(self, job_container):
        #with self.dag_lock:
        if True:
            failed_job_containers = set([])
            tester = job_container.getTester()
            job_dag = job_container.getDAG()
            if (tester.isFinished() and not tester.didPass() and not tester.isSilent() and not self.skipPrereqs()) \
                or (self.options.dry_run and not tester.isSilent()):

                # Ask the DAG to delete and return the downstream jobs associated with this job
                failed_job_containers.update(job_dag.delete_downstreams(job_container))

            for failed_job in failed_job_containers:
                tester = failed_job.getTester()
                tester.setStatus('skipped dependency', tester.bucket_skip)

        return failed_job_containers

    # Iterate over supplied tester data container dict and add edge nodes to the DAG (create dependencies)
    def createDependencies(self, job_container_dict):
        for tester_name, job_container in job_container_dict.iteritems():
            tester = job_container.getTester()
            job_dag = job_container.getDAG()

            # Attempt to add this node to the DAG
            job_dag.add_node_if_not_exists(job_container)

            # Create the edge dependency DAG
            for prereq in tester.getPrereqs():

                if prereq in job_container_dict.keys():

                    # Attempt to add the prereq node if not already exists
                    job_dag.add_node_if_not_exists(job_container_dict[prereq])

                    # Attempt to create an edge node dependency
                    try:
                        job_dag.add_edge(job_container_dict[prereq], job_container)

                    # We do not care about cyclic errors, they have been handled already
                    except dag.DAGValidationError:
                        pass

        return job_container_dict

    # Method to return failing testers which can only be discovered when scrutinized as a group (race conditions
    # for example)
    def checkGroupFailures(self, job_container_dict):
        temp_dag = dag.DAG()
        failed_or_skipped_testers = set([])

        # Create DAG independent nodes
        for tester_name, job_container in job_container_dict.iteritems():
            tester = job_container.getTester()

            # If this tester is not runnable, continue to the next tester
            if tester.getRunnable(self.options):

                # Add parent node
                temp_dag.add_node_if_not_exists(job_container)

            else:
                failed_or_skipped_testers.add(tester)
                continue

        # Create edge nodes
        for tester_name, job_container in job_container_dict.iteritems():
            tester = job_container.getTester()

            # Add the prereq node and edges
            for prereq in tester.getPrereqs():

                try:
                    # This may produce a KeyError (unknown dependency)
                    prereq_tester = job_container_dict[prereq].getTester()

                    # Create the edge node (can create either a DAG validation error 'cyclic' or DAG edge not
                    # existing error 'skipped dependency')
                    temp_dag.add_edge(job_container_dict[prereq], job_container)

                # Skipped Dependencies
                except dag.DAGEdgeIndError:
                    if not self.skipPrereqs():
                        tester.setStatus('skipped dependency', tester.bucket_skip)
                        failed_or_skipped_testers.add(tester)

                    # Add the parent node / dependency edge to create a functional DAG now that we have caught
                    # the skipped dependency (needed for discovering race conditions later on)
                    temp_dag.add_node_if_not_exists(job_container_dict[prereq])
                    temp_dag.add_edge(job_container_dict[prereq], job_container)

                # Cyclic Failure
                except dag.DAGValidationError:
                    tester.setStatus('Cyclic or Invalid Dependency Detected!', tester.bucket_fail)
                    failed_or_skipped_testers.add(tester)

                # Unknown Dependency Failure
                except KeyError:
                    tester.setStatus('unknown dependency', tester.bucket_fail)
                    failed_or_skipped_testers.add(tester)

                # Skipped/Silent/Deleted Testers fall into this catagory, caused by 'job_container' being skipped
                # during the first iteration above
                except dag.DAGEdgeDepError:
                    pass

        # With a working DAG created above (even a partial one), discover race conditions with remaining runnable
        # testers.
        while temp_dag.size():
            output_files_in_dir = set()

            # Get a list of concurrent tester containers
            concurrent_jobs = temp_dag.ind_nodes()

            for job_container in concurrent_jobs:
                tester = job_container.getTester()
                output_files = tester.getOutputFiles()

                # check if we have colliding output files
                if len(output_files_in_dir.intersection(set(output_files))):

                    # Fail this concurrent group of testers
                    for this_job in concurrent_jobs:
                        tester = this_job.getTester()
                        tester.setStatus('OUTFILE RACE CONDITION', tester.bucket_fail)
                        failed_or_skipped_testers.add(tester)

                    # Break out of initial concurrent_jobs and allow checking of the next concurrent group
                    break

                output_files_in_dir.update(output_files)

            # Delete this group of job containers and allow the loop to continue
            for job_container in concurrent_jobs:
                temp_dag.delete_node(job_container)

        # Return any failed or skipped testers discovered
        return failed_or_skipped_testers

    # Loop through the testers and generate a Tester Data container (AKA 'job') which we will pass to the thread
    # pools
    def schedule(self, testers):
        # Increment our simple queue count with the number of testers the scheduler received
        with self.slot_lock:
            self.job_queue_count += len(testers)

        # Create the job DAG object we will share between this group of testers
        job_dag = dag.DAG()

        # Create a local storage of tester names to job container map
        name_to_job_container = {}
        for tester in testers:
            name_to_job_container[tester.getTestName()] = TesterData(tester, job_dag, self.options)

            # Add this TesterData to a set. This is used in the event of a KeyboardInterrupt to iterate over and
            # kill its subprocess
            self.tester_datas.add(name_to_job_container[tester.getTestName()])

        # Discover failures only prevalent when scrutinizing the entire group as a whole
        # Note: this also returns non-failing skipped testers, because its necessary to know about them when
        # discovering things like race conditions. It may not belong in that method, but its efficient to perform
        # "while we're here".
        skipped_or_failed_testers = self.checkGroupFailures(name_to_job_container)

        # Local storage of skipped/failed job containers
        non_runnable_jobs = set([])

        # Create a set of failing job containers
        for failed_tester in skipped_or_failed_testers:
            non_runnable_jobs.add(name_to_job_container[failed_tester.getTestName()])

        # Create edge nodes (dependencies) in the DAG shared between this group of testers
        self.createDependencies(name_to_job_container)

        # Iterate over the items in our non_runnable_jobs and handle any skipped dependencies
        for job in non_runnable_jobs.copy():
            # Handle any downstream job dependencies this job will create by being skipped
            additionally_skipped = self.processDownstreamTests(name_to_job_container[job.getTestName()])

            # update our set of non_runnable_jobs with downstreams that also can not run
            non_runnable_jobs.update(additionally_skipped)

            # Delete _this_ non runnable job from the DAG
            job_dag.delete_node_if_exists(name_to_job_container[job.getTestName()])

        # Get a count of all the items still in the DAG. This will be the jobs that ultimately get queued
        runnable_jobs = job_dag.size()

        # Accountability is a good thing. Make sure we didn't drop a tester somehow.
        if len(non_runnable_jobs) + runnable_jobs != len(testers):
            raise SchedulerError('Runnable tests in addition to Skipped tests does not match total scheduled test count!')

        # Assign a status thread to begin work on any skipped/failed jobs
        self.queueJobs(status_job_containers=non_runnable_jobs)

        # Build our list of runnable jobs and set the tester status to queued
        job_list = []
        if runnable_jobs:
            job_list = job_dag.ind_nodes()
            for job_container in job_list:
                tester = job_container.getTester()
                tester.setStatus('QUEUED', tester.bucket_pending)

        # Queue runnable jobs
        self.queueJobs(tester_job_containers=job_list)

    # Block until all jobs are finished
    def waitFinish(self):
        while self.job_queue_count > 0:
            sleep(0.5)

        # Wait for the tester pool to empty before waiting on the status pool. The tester pool and status pool can
        # exhange jobs between each other but it is always the status pool that will have items in the queue last
        # (that last job with a finished status to be printed)

        # Close the tester_pool so jobs can no longer be scheduled for launching
        self.tester_pool.close()

        # Wait for the tester_pool to empty
        self.tester_pool.join()

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
                                                 self.handleLongRunningJobs,
                                                 (start_timers,))
            long_running_timer.start()

            # Failed TIMEOUT timer
            timeout_timer = threading.Timer(float(tester.getMaxTime()),
                                          self.handleTimeoutJobs,
                                          (start_timers,))
            timeout_timer.start()

            return (long_running_timer, timeout_timer)

        # Stop any timers we started
        elif stop_timers:
            for timer in stop_timers:
                timer.cancel()

    # Handle long running jobs
    def handleLongRunningJobs(self, job_container):
        tester = job_container.getTester()
        tester.specs.addParam('caveats', ['FINISHED'], "")
        tester.setStatus('RUNNING...', tester.bucket_pending)
        self.queueJobs(status_job_containers=[job_container])

    # Handle jobs that are timing out
    def handleTimeoutJobs(self, job_container):
        tester = job_container.getTester()
        tester.setStatus('TIMEOUT', tester.bucket_fail)
        job_container.killProcess()

    # return load average
    def getLoad(self):
        loadAverage = 0.0
        try:
            loadAverage = os.getloadavg()[0]
        except AttributeError:
            pass      # getloadavg() not available in this implementation of os
        return loadAverage

    def satisfyLoad(self):
        # We'll always run at least one job regardless of load or we'll starve!
        while self.slots_in_use > 1 and self.getLoad() >= self.average_load:
            sleep(1.0)

    # Allocate requested slots and return bool if we are able to run this job
    def reserveSlots(self, job_container):
        tester = job_container.getTester()

        # Make sure we are complying with the requested load average
        self.satisfyLoad()

        with self.slot_lock:
            can_run = False
            # See if we can fit this job into our busy schedule
            if self.slots_in_use + tester.getProcs(self.options) <= self.available_slots:
                can_run = True

            # Check for insufficient slots -soft limit
            # TODO: Create a unit test for this case
            elif tester.getProcs(self.options) > self.available_slots and self.soft_limit:
                tester.specs.addParam('caveats', ['OVERSIZED'], "")
                can_run = True

            # Check for insufficient slots -hard limit (skip this job)
            # TODO: Create a unit test for this case
            elif tester.getProcs(self.options) > self.available_slots and not self.soft_limit:
                # Set status for _this_ tester for the specific caveat
                tester.setStatus('insufficient slots', tester.bucket_skip)
                can_run = False

            if can_run:
                # Record the amount of slots we're about to consume
                self.slots_in_use += tester.getProcs(self.options)

        return can_run

    # Remove incoming job and get next available set of jobs
    def getNextJobGroup(self, job_container):
        with self.dag_lock:
            tester = job_container.getTester()
            job_dag = job_container.getDAG()
            next_job_list = []

            # Delete this job from the shared DAG
            job_dag.delete_node(job_container)

            # Get next available job list
            concurrent_jobs = job_dag.ind_nodes()

            for next_job_container in concurrent_jobs:
                queued_tester = next_job_container.getTester()

                # Verify this job is not already running/pending/skipped
                if queued_tester.isInitialized():
                    # Set this next new job to pending so as to prevent this job from being launched a second time
                    queued_tester.setStatus('QUEUED', queued_tester.bucket_pending)
                    next_job_list.append(next_job_container)

        return next_job_list

    # Queue a job to a specified thread pool
    def queueJobs(self, status_job_containers=[], tester_job_containers=[]):
        for job in tester_job_containers:
            # Assign a thread to do a test
            self.tester_pool.apply_async(self.testWorker, (job,))

        for job in status_job_containers:
            # Assign a thread to print some status
            self.status_pool.apply_async(self.statusWoker, (job,))

    # Status processing (inform TestHarness of a status)
    def statusWoker(self, job_container):
        tester = job_container.getTester()

        # Return to the TestHarness
        self.harness.handleTestStatus(job_container)

        # Decrement the job queue count now that this test has finished
        if tester.isFinished():
            with self.slot_lock:
                self.job_queue_count -= 1

    # Test execution processing
    def testWorker(self, job_container):
        tester = job_container.getTester()

        # Check if we have available slots
        if self.reserveSlots(job_container):

            # Get a timer tuple of started thread timers
            my_timers = self.handleTimers(start_timers=job_container)

            # Call derived run method outside of the thread_lock (run is blocking)
            self.run(job_container)

            # The test finished, so stop any timers we received when starting them
            self.handleTimers(stop_timers=my_timers)

            # Derived run should never set a pending status. If it did, something is wrong as the scheduler can not
            # handle this scenario
            if tester.isPending():
                raise SchedulerError('Derived Scheduler can not return a pending status!')

            # Queue this job for status printing
            self.queueJobs(status_job_containers=[job_container])

            # With this job completed, determine if it failed and if any dependencies will fail because of it
            failed_downstream = self.processDownstreamTests(job_container)

            # Queue any failing downstream jobs for status printing (skipped dependency)
            self.queueJobs(status_job_containers=failed_downstream)

            # Get the next group of jobs we can run
            next_job_group = self.getNextJobGroup(job_container)

            # Recover worker count before attempting to queue more jobs
            with self.slot_lock:
                self.slots_in_use = max(0, self.slots_in_use - tester.getProcs(self.options))

            # Queue this new batch of runnable jobs
            self.queueJobs(tester_job_containers=next_job_group)

        # Not enough slots to run the job, currently
        else:
            # There will never be enough slots to run this job (insufficient slots)
            if tester.isFinished():
                # Set 'skipped dependency' status on downstream testers
                failed_downstream = self.processDownstreamTests(job_container)
                failed_downstream.add(job_container)
                self.queueJobs(status_job_containers=failed_downstream)
            else:
                # There are no available slots, currently. Place back in queue, and sleep for a bit
                self.queueJobs(tester_job_containers=[job_container])
                sleep(0.3)
