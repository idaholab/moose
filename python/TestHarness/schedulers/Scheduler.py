#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from time import sleep
from FactorySystem.MooseObject import MooseObject
from Job import Job
import os, traceback
from contrib import dag
from timeit import default_timer as clock

from multiprocessing.pool import ThreadPool
import threading # for thread locking and thread timers

class SchedulerError(Exception):
    pass

class Scheduler(MooseObject):
    """
    Base class for handling jobs asynchronously. To use this class, call .schedule()
    and supply a list of testers to schedule. Each group of testers supplied will begin
    running immediately.

    Syntax:
       .schedule([list of tester objects])

    A list of testers will be added to a queue and begin calling their derived run method.
    You can continue to add more testers to the queue in this fashion.

    Once you schedule all the testers you wish to test, call .waitFinish() to wait until
    all testers have finished.

    """

    @staticmethod
    def validParams():
        params = MooseObject.validParams()
        params.addRequiredParam('average_load',  64.0, "Average load to allow")
        params.addRequiredParam('max_processes', None, "Hard limit of maxium processes to use")

        return params

    # This is what will be checked for when we look for valid schedulers
    IS_SCHEDULER = True

    def __init__(self, harness, params):
        MooseObject.__init__(self, harness, params)

        ## The test harness to run callbacks on
        self.harness = harness

        # Retrieve and store the TestHarness options for use in this object
        self.options = harness.getOptions()

        # The Scheduler class can be initialized with no "max_processes" argument and it'll default
        # to a soft limit. If however a max_processes is passed we'll treat it as a hard limit.
        # The difference is whether or not we allow single jobs to exceed the number of slots.
        if params['max_processes'] == None:
            self.available_slots = 1
            self.soft_limit = True
        else:
            self.available_slots = params['max_processes'] # hard limit
            self.soft_limit = False

        # Requested average load level to stay below
        self.average_load = params['average_load']

        # The time the status queue reported no activity to the TestHarness
        self.last_reported = clock()

        # A set containing jobs that have been reported
        self.jobs_reported = set([])

        # Initialize run_pool based on available slots
        self.run_pool = ThreadPool(processes=self.available_slots)

        # Initialize status_pool to only use 1 process (to prevent status messages from getting clobbered)
        self.status_pool = ThreadPool(processes=1)

        # Slot Lock when processing resource allocations
        self.slot_lock = threading.Lock()

        # DAG Lock when processing the DAG
        self.dag_lock = threading.Lock()

        # Workers in use (single job might request multiple slots)
        self.slots_in_use = 0

        # Jobs waiting to finish (includes actively running jobs)
        self.job_queue_count = 0

        # Set containing our Job containers. We use this in the event of a KeyboardInterrupt to
        # iterate over and kill any subprocesses
        self.tester_datas = set([])

        # Allow threads to set an exception state
        self.error_state = False

    def killRemaining(self):
        """
        Method to kill any running subprocess started by the Scheduler. This also
        closes the status pool to prevent further statuses from printing to the
        screen.
        """
        self.run_pool.close()
        self.status_pool.close()

        for tester_data in self.tester_datas:
            tester_data.killProcess()
        self.job_queue_count = 0

    def schedulerError(self):
        """
        If any runWorker, statusWorker threads caused an exception, this method
        will admit to it.
        """
        return self.error_state

    def reportSkipped(self, jobs):
        """
        Allow derived schedulers to do something with skipped jobs
        """
        return

    def preLaunch(self, job_dag):
        """
        Allow derived schedulers to modify the DAG before jobs are launched
        """
        return

    def run(self, job_container):
        """ Call derived run method """
        return

    def postRun(self, job_container):
        """
        Allow derived schdulers to perform post run methods on job
        """
        return

    def cleanUp(self):
        """ Allow derived schedulers to perform cleanup operations """
        return

    def notifyFinishedSchedulers(self):
        """ Notify derived schedulers we are finished """
        return

    def skipPrereqs(self):
        """
        Method to return boolean to skip dependency prerequisites checks.
        """
        if self.options.ignored_caveats:
            if 'all' in self.options.ignored_caveats or 'prereq' in self.options.ignored_caveats:
                return True
        return False

    def processDownstreamTests(self, job_container):
        """
        Method to discover and delete downstream jobs due to supplied job failing.
        """
        with self.dag_lock:
            failed_job_containers = set([])
            tester = job_container.getTester()
            job_dag = job_container.getDAG()
            if (tester.isFinished() and not tester.didPass() and not tester.isSilent() and not self.skipPrereqs()) \
               and not tester.isQueued() \
               or (self.options.dry_run and not tester.isSilent()):

                # Ask the DAG to delete and return the downstream jobs associated with this job
                failed_job_containers.update(job_dag.delete_downstreams(job_container))

            for failed_job in failed_job_containers:
                failed_tester = failed_job.getTester()
                failed_tester.addCaveats('skipped dependency')
                failed_tester.setStatus(failed_tester.bucket_skip.status, failed_tester.bucket_skip)

        return failed_job_containers

    def buildDAG(self, job_container_dict, job_dag):
        """
        Build the DAG and catch any failures.
        """

        failed_or_skipped_testers = set([])

        # Create DAG independent nodes
        for tester_name, job_container in job_container_dict.iteritems():
            tester = job_container.getTester()

            # If this tester is not runnable, continue to the next tester
            if tester.getRunnable(self.options):

                job_dag.add_node_if_not_exists(job_container)

            else:
                failed_or_skipped_testers.add(tester)
                continue

        # Create edge nodes
        for tester_name, job_container in job_container_dict.iteritems():
            tester = job_container.getTester()

            # Add the prereq node and edges
            for prereq in tester.getPrereqs():

                try:
                    # Try to produce a KeyError and capture an unknown dependency
                    job_container_dict[prereq]

                    # Try to produce either a cyclic or skipped dependency error using the DAG's
                    # built-in exception methods
                    job_dag.add_edge(job_container_dict[prereq], job_container)

                # Skipped Dependencies
                except dag.DAGEdgeIndError:
                    if not self.skipPrereqs():
                        if self.options.reg_exp:
                            tester.addCaveats('dependency does not match re')
                            tester.setStatus(tester.bucket_skip.status, tester.bucket_skip)
                        else:
                            tester.addCaveats('skipped dependency')
                            tester.setStatus(tester.bucket_skip.status, tester.bucket_skip)
                        failed_or_skipped_testers.add(tester)

                    # Add the parent node / dependency edge to create a functional DAG now that we have caught
                    # the skipped dependency (needed for discovering race conditions later on)
                    job_dag.add_node_if_not_exists(job_container_dict[prereq])
                    job_dag.add_edge(job_container_dict[prereq], job_container)

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
        failed_or_skipped_testers.update(self.checkRaceConditions(job_dag))

        return failed_or_skipped_testers

    def checkRaceConditions(self, dag_object):
        """
        Return a set of failing testers exhibiting race conditions with their
        output file.
        """
        failed_or_skipped_testers = set([])

        # clone the dag so we can operate destructively on the cloned dag
        dag_clone = dag_object.clone()

        while dag_clone.size():
            output_files_in_dir = set()

            # Get a list of concurrent job containers
            concurrent_jobs = dag_clone.ind_nodes()

            for job_container in concurrent_jobs:
                tester = job_container.getTester()
                output_files = tester.getOutputFiles()

                # check if we have colliding output files
                if len(output_files_in_dir.intersection(set(output_files))):

                    # Fail this concurrent group of testers
                    for this_job in concurrent_jobs:
                        failed_tester = this_job.getTester()
                        failed_tester.setStatus('OUTFILE RACE CONDITION', tester.bucket_fail)
                        failed_or_skipped_testers.add(failed_tester)

                    # collisions detected, move on to the next set
                    break

                output_files_in_dir.update(output_files)

            # Delete this group of job containers and allow the loop to continue
            for job_container in concurrent_jobs:
                dag_clone.delete_node(job_container)

        return failed_or_skipped_testers

    def schedule(self, testers):
        """
        Schedule supplied list of testers for execution.
        """
        # If any threads caused an exception, we have already closed down the queue and need to
        # not schedule any more jobs
        if self.run_pool._state or self.error_state:
            return

        # Instance the DAG class so we can share it amongst all the Job containers
        job_dag = dag.DAG()

        non_runnable_jobs = set([])
        name_to_job_container = {}

        # Increment our simple queue count with the number of testers the scheduler received
        with self.slot_lock:
            self.job_queue_count += len(testers)

        # Create a local dictionary of tester names to job containers. Add this dictionary to a
        # set. We will use this set as a way to gain access to their methods.
        for tester in testers:
            if tester.getTestName() in name_to_job_container:
                tester.addCaveats('duplicate test')
                tester.setStatus(tester.bucket_skip.status, tester.bucket_skip)
                non_runnable_jobs.add(Job(tester, job_dag, self.options))
            else:
                name_to_job_container[tester.getTestName()] = Job(tester, job_dag, self.options)
                self.tester_datas.add(name_to_job_container[tester.getTestName()])

        # Populate job_dag with testers. This method will also return any testers which caused failures
        # while building the DAG.
        skipped_or_failed_testers = self.buildDAG(name_to_job_container, job_dag)

        # Create a set of failing job containers
        for failed_tester in skipped_or_failed_testers:
            non_runnable_jobs.add(name_to_job_container[failed_tester.getTestName()])

        # Iterate over the jobs in our non_runnable_jobs and handle any downstream jobs affected by
        # 'job'. These will be our 'skipped dependency' tests.
        for job in non_runnable_jobs.copy():
            additionally_skipped = self.processDownstreamTests(job)
            non_runnable_jobs.update(additionally_skipped)
            job_dag.delete_node_if_exists(job)

        # Get a count of all the items still in the DAG. These will be the jobs that ultimately are queued
        runnable_jobs = job_dag.size()

        # Make sure we didn't drop a tester somehow
        if len(non_runnable_jobs) + runnable_jobs != len(testers):
            raise SchedulerError('Runnable tests in addition to Skipped tests does not match total scheduled test count!')

        # Inform derived schedulers of the jobs we are skipping immediately
        self.reportSkipped(non_runnable_jobs)

        # Assign a status thread to begin work on any skipped/failed jobs
        self.queueJobs(status_jobs=non_runnable_jobs)

        # Allow derived schedulers to modify the dag before we launch
        # TODO: We don't like this, and this will change when we move to better DAG handling.
        if runnable_jobs:
            self.preLaunch(job_dag)

        # Build our list of runnable jobs and set the tester's status to queued
        job_list = []
        if runnable_jobs:
            job_list = job_dag.ind_nodes()
            for job_container in job_list:
                tester = job_container.getTester()
                tester.setStatus('QUEUED', tester.bucket_pending)

        # Queue runnable jobs
        self.queueJobs(run_jobs=job_list)

    def waitFinish(self):
        """
        Block while the job queue is not empty. Once empty, this method will begin closing down
        the thread pools and perform a join. Once the last thread exits, we return from this
        method.

        There are two thread pools in play; the Tester pool which is performing all the tests,
        and the Status pool which is handling the printing of tester statuses. Because the
        Status pool will always have the last item needing to be 'printed', we close and join
        the Tester pool first, and then we do the same to the Status pool.
        """
        while self.job_queue_count > 0:
            # One of our children died :( so exit uncleanly
            if self.error_state:
                self.killRemaining()
                return
            sleep(0.5)

        self.run_pool.close()
        self.run_pool.join()
        self.status_pool.close()
        self.status_pool.join()

        # Notify derived schedulers we are exiting
        self.notifyFinishedSchedulers()

    def handleLongRunningJobs(self, job_container):
        """ Handle jobs that have not reported in alotted time """
        if job_container not in self.jobs_reported:
            report = False
            with self.dag_lock:
                tester = job_container.getTester()
                if not tester.isFinished():
                    report = True
                    tester.setStatus('RUNNING...', tester.bucket_pending)
            if report:
                self.queueJobs(status_jobs=[job_container])

            # Restart the reporting timer for this job
            job_container.report_timer = threading.Timer(float(tester.getMinReportTime()),
                                                         self.handleLongRunningJobs,
                                                         (job_container,))

            job_container.report_timer.start()

    def handleTimeoutJobs(self, job_container):
        """ Handle jobs that have timed out """
        tester = job_container.getTester()
        tester.setStatus('TIMEOUT', tester.bucket_fail)
        job_container.killProcess()

    def getLoad(self):
        """ Method to return current load average """
        loadAverage = 0.0
        try:
            loadAverage = os.getloadavg()[0]
        except AttributeError:
            pass      # getloadavg() not available in this implementation of os
        return loadAverage

    def satisfyLoad(self):
        """ Method for controlling load average """
        while self.slots_in_use > 1 and self.getLoad() >= self.average_load:
            sleep(1.0)

    def reserveSlots(self, job_container):
        """
        Method which allocates resources to perform the job. Returns bool if job
        should be allowed to run based on available resources.
        """
        tester = job_container.getTester()

        # comply with load average
        if self.options.load:
            self.satisfyLoad()

        with self.slot_lock:
            can_run = False
            if self.slots_in_use + job_container.getSlots() <= self.available_slots:
                can_run = True

            # Check for insufficient slots -soft limit
            elif job_container.getSlots() > self.available_slots and self.soft_limit:
                tester.addCaveats('OVERSIZED')
                can_run = True

            # Check for insufficient slots -hard limit (skip this job)
            # TODO: Create a unit test for this case
            elif job_container.getSlots() > self.available_slots and not self.soft_limit:
                tester.addCaveats('insufficient slots')
                tester.setStatus(tester.bucket_skip.status, tester.bucket_skip)

            if can_run:
                self.slots_in_use += job_container.getSlots()

        return can_run

    def getNextJobGroup(self, job_dag):
        """
        Prepare and return a list of concurrent runnable jobs
        """
        with self.dag_lock:
            next_job_list = []

            # Get concurrent available job list
            concurrent_jobs = job_dag.ind_nodes()

            for job_container in concurrent_jobs:
                tester = job_container.getTester()

                # Verify this job is not already running/pending/skipped
                if tester.isInitialized():
                    # Set this next new job to pending so as to prevent this job from being launched a second time
                    tester.setStatus('QUEUED', tester.bucket_pending)
                    next_job_list.append(job_container)

        return next_job_list

    def queueJobs(self, status_jobs=[], run_jobs=[]):
        """
        Method to control which thread pool jobs enter.
        Syntax:

           To have a job(s) display its current status to the screen:
           .queueJobs(status_jobs=[job_container_list]

           To begin running job(s):
           .queueJobs(run_jobs=[job_container_list]

        """
        for job_container in run_jobs:
            if not self.error_state or not self.error_state:
                self.run_pool.apply_async(self.runWorker, (job_container,))

        for job_container in status_jobs:
            if not self.status_pool._state or not self.error_state:
                self.status_pool.apply_async(self.statusWorker, (job_container,))

    def statusWorker(self, job_container):
        """ Method the status_pool calls when an available thread becomes ready """
        # Wrap entire statusWorker thread inside a try/exception to catch thread errors
        try:
            tester = job_container.getTester()

            # If the job is still running for a long period of time and we have not reported
            # this same job alread, report it now.
            if tester.isPending():
                if clock() - self.last_reported >= float(tester.getMinReportTime()) and job_container not in self.jobs_reported:
                    # Inform the TestHarness of a long running test (RUNNING...)
                    self.harness.handleTestStatus(job_container)

                    # ...And then set the finished caveat now that the running status has printed
                    tester.addCaveats('FINISHED')

                    # Add this job to the reported container so it does not happen again
                    self.jobs_reported.add(job_container)

                # Job is 'Pending', but is under the threshold to be reported (return now so
                # last_reported time does not get updated). This will ensure that if nothing
                # has happened between 'now' and another occurrence of our thread timer event
                # we do report it.
                else:
                    return

            else:
                # All other statuses are sent unmolested
                self.harness.handleTestStatus(job_container)

            # Decrement the job queue count now that this job has finished
            if tester.isFinished():
                with self.slot_lock:
                    self.job_queue_count -= 1

            # Record current reported time only if it is an activity the user will see
            if not tester.isSilent() or not tester.isDeleted():
                self.last_reported = clock()

        except Exception:
            self.error_state = True
            print('statusWorker Exception: %s' % (traceback.format_exc()))

    def runWorker(self, job_container):
        """ Method the run_pool calls when an available thread becomes ready """
        # Wrap the entire runWorker thread inside a try/exception to catch thread errors
        try:
            tester = job_container.getTester()
            # Check if there are enough resources to run this job
            if self.reserveSlots(job_container):

                # Start long running timer
                job_container.report_timer = threading.Timer(float(tester.getMinReportTime()),
                                                             self.handleLongRunningJobs,
                                                             (job_container,))
                job_container.report_timer.start()

                # Start timeout timer
                timeout_timer = threading.Timer(float(tester.getMaxTime()),
                                          self.handleTimeoutJobs,
                                          (job_container,))
                timeout_timer.start()

                # Call the derived run method (blocking)
                self.run(job_container)

                # Stop timers now that the job has finished on its own
                job_container.report_timer.cancel()
                timeout_timer.cancel()

                # Derived run needs to set a non-pending status of some sort.
                if tester.isPending():
                    raise SchedulerError('Derived Scheduler %s can not return a pending status!' % (self.__class__))

                # Determin if this job creates any skipped dependencies (if it failed), and send
                # this new list of jobs to the status queue to be printed.
                possibly_skipped_job_containers = self.processDownstreamTests(job_container)
                possibly_skipped_job_containers.add(job_container)
                self.queueJobs(status_jobs=possibly_skipped_job_containers)

                # Delete this job from the shared DAG while the DAG is locked
                with self.dag_lock:
                    job_dag = job_container.getDAG()
                    job_dag.delete_node(job_container)

                # Get next job list
                next_job_group = self.getNextJobGroup(job_dag)

                # Allow derived schedulers to perform post run operations
                self.postRun(job_container)

                # Recover worker count before attempting to queue more jobs
                with self.slot_lock:
                    self.slots_in_use = max(0, self.slots_in_use - job_container.getSlots())

                # Queue this new batch of runnable jobs
                self.queueJobs(run_jobs=next_job_group)

            # Not enough slots to run the job, currently
            else:
                # There will never be enough slots to run this job (insufficient slots)
                if tester.isFinished():
                    failed_downstream = self.processDownstreamTests(job_container)
                    failed_downstream.add(job_container)
                    self.queueJobs(status_jobs=failed_downstream)

                # There are no available slots, currently. Place back in queue, and sleep for a bit
                else:
                    self.queueJobs(run_jobs=[job_container])
                    sleep(0.3)

        except Exception:
            self.error_state = True
            print('runWorker Exception: %s' % (traceback.format_exc()))
