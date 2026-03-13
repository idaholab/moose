# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html
import os
import sys
import threading
import traceback
from dataclasses import dataclass
from enum import Enum
from multiprocessing.pool import ThreadPool
from time import sleep
from timeit import default_timer as clock
from typing import TYPE_CHECKING, Optional

import pyhit
from FactorySystem.MooseObject import MooseObject

from TestHarness.JobDAG import JobDAG
from TestHarness.mpi_config import (
    MPIConfig,
    build_hwloc_topology,
    get_mpi_config,
)
from TestHarness.StatusSystem import StatusSystem
from TestHarness.util import findBSDTime, findGNUTime, outputHeader

if TYPE_CHECKING:
    from TestHarness.schedulers.Job import Job


class SchedulerError(Exception):
    pass


class TimeUtilityType(Enum):
    """The type of time utility found, if any."""

    BSD = 0
    GNU = 1
    NONE = 2


@dataclass(frozen=True)
class SchedulerOptions:
    """Options for the Scheduler."""

    hwloc_topology_path: Optional[str]
    """Path to the pre-built hwloc topology file, if any."""

    mpi_config: MPIConfig
    """The MPI configuration."""

    monitor_job_cpu: bool
    """Whether or not to monitor Job CPU usage."""
    monitor_job_memory: bool
    """Whether or not to monitor Job memory usage."""

    time_utility_path: Optional[str]
    """The path to the time utility found, if any."""
    time_utility_type: TimeUtilityType
    """The type of time utility found, if any."""


class Scheduler(MooseObject):
    """
    Base class for handling jobs asynchronously. To use this class, call .schedule()
    and supply a list of testers to schedule. Each group of testers supplied will begin
    running immediately.

    Syntax:
       .schedule([list of tester objects])

    A list of testers will be added to a queue and begin calling their derived run method.
    You can continue to add more testers to the queue in this fashion.

    Once all jobs have been scheduled, call .waitFinish() to wait until all jobs have
    finished.
    """

    @staticmethod
    def validParams():
        params = MooseObject.validParams()
        params.addRequiredParam("average_load", 64.0, "Average load to allow")
        params.addRequiredParam(
            "max_processes", None, "Hard limit of maxium processes to use"
        )
        params.addParam(
            "min_reported_time",
            10,
            "The minimum time elapsed before a job is reported as taking to long to run.",
        )

        return params

    # This is what will be checked for when we look for valid schedulers
    IS_SCHEDULER = True

    CAN_SET_HWLOC_TOPOLOGY = False
    """Whether or not to set hwloc topology if available."""
    CAN_SET_MAX_MEMORY = False
    """Whether or not Job max memory can be set (Jobs can be killed if over memory)."""
    CAN_OPENMPI_OVERSUBSCRIBE = False
    """Whether or not OpenMPI can be set to oversubscribe."""
    MONITOR_JOB_CPU = False
    """Whether or not this Scheduler should monitor Job process CPU usage."""
    MONITOR_JOB_MEMORY = False
    """Whether or not this Scheduler should monitor Job process memory usage."""

    def __init__(self, harness, params):
        MooseObject.__init__(self, harness, params)

        ## The test harness to run callbacks on
        self.harness = harness

        # Retrieve and store the TestHarness options for use in this object
        self.options = harness.getOptions()

        # The Scheduler class can be initialized with no "max_processes" argument and it'll default
        # to a soft limit. If however a max_processes is passed we'll treat it as a hard limit.
        # The difference is whether or not we allow single jobs to exceed the number of slots.
        self.available_slots, self.soft_limit = self.availableSlots(params)

        self.average_load = params["average_load"]

        self.min_report_time = params["min_reported_time"]

        # Initialize run_pool based on available slots
        self.run_pool = ThreadPool(processes=self.available_slots)

        # Initialize status_pool to only use 1 process (to prevent status messages from getting clobbered)
        self.status_pool = ThreadPool(processes=1)

        # Slot lock when processing resource allocations and modifying slots_in_use
        self.slot_lock = threading.Lock()

        # A combination of processors + threads (-j/-n) currently in use, that a job requires
        self.slots_in_use = 0

        # List of Lists containing all scheduled jobs
        self.__scheduled_jobs = []

        # Set containing all job objects entering the run_pool
        self.__job_bank = set([])

        # List of lists containing all job objects entering the run_pool
        self.__dag_bank = []

        # Lock for __job_bank and __dag_bank
        self.__bank_lock = threading.Lock()

        # Total running Job and Test failures encountered
        self.__failures = 0

        # Allow threads to set a global exception
        self.__error_state = False

        # Lock for __error_state
        self.__error_state_lock = threading.Lock()

        # Private set of jobs currently running
        self.__active_jobs = set([])

        # Lock for __active_jobs
        self.__active_jobs_lock = threading.Lock()

        # Jobs that are taking longer to finish than the alloted time are reported back early to inform
        # the user 'stuff' is still running. Jobs entering this set will not be reported again.
        self.jobs_reported = set([])

        # Lock for jobs_repoted
        self.jobs_reported_lock = threading.Lock()

        # The last time the scheduler reported something
        self.last_reported_time = clock()

        # Whether or not to report long running jobs as RUNNING
        self.report_long_jobs = True
        # Whether or not to enforce the timeout of jobs
        self.enforce_timeout = True

        self.scheduler_options: SchedulerOptions = self.buildSchedulerOptions(
            self.options
        )
        """The options for the scheduler."""

    def buildSchedulerOptions(self, options) -> SchedulerOptions:
        """Build the SchedulerOptions for this scheduler."""
        # Build MPI configuration
        mpi_config = get_mpi_config()

        # Whether or not to monitor Job resources, depending on static
        # Scheduler options and runtime command line options
        monitor_job_cpu: bool = (
            self.MONITOR_JOB_CPU is True and not options.no_cpu_tracking
        )
        monitor_job_memory: bool = (
            self.MONITOR_JOB_MEMORY is True and not options.no_memory_tracking
        )

        # Find the time utility if needed, disabling CPU tracking
        # if enabled but no time utility available
        time_utility_type = TimeUtilityType.NONE
        time_utility_path: Optional[str] = None
        if monitor_job_cpu:
            if time_utility_path := findGNUTime():
                time_utility_type = TimeUtilityType.GNU
            elif time_utility_path := findBSDTime():
                time_utility_type = TimeUtilityType.BSD
            if time_utility_type == TimeUtilityType.NONE:
                monitor_job_cpu = False

        # Build hwloc topology file if set to do so
        hwloc_topology_path: Optional[str] = None
        if (
            self.CAN_SET_HWLOC_TOPOLOGY
            and mpi_config.hwloc
            and not options.no_hwloc_topology
        ):
            if hwloc_topology_path := build_hwloc_topology():
                self.harness.printInfo(
                    f"Using cached hwloc topology in '{hwloc_topology_path}'"
                )

        # Can't restrict resources without tracking
        if self.options.max_cpu_per_slot and not monitor_job_cpu:
            self.harness.errorExit(
                "Cannot specify --max-cpu-per-slot; CPU tracking is not available"
            )
        if self.options.max_memory_per_slot and (
            not monitor_job_memory or not self.CAN_SET_MAX_MEMORY
        ):
            self.harness.errorExit(
                "Cannot specify --max-memory-per-slot; memory tracking is not available"
            )
        # Can't disable hwloc topology
        if options.no_hwloc_topology and not self.CAN_SET_HWLOC_TOPOLOGY:
            self.harness.errorExit(
                "Cannot --no-hwloc-topology; scheduler does not " "support setting it"
            )
        # Can't disable openmpi oversubscribe
        if options.no_openmpi_oversubscribe and not self.CAN_OPENMPI_OVERSUBSCRIBE:
            self.harness.errorExit(
                "Cannot --no-openmpi-oversubscribe; scheduler does not "
                "support setting it"
            )

        return SchedulerOptions(
            hwloc_topology_path=hwloc_topology_path,
            mpi_config=mpi_config,
            monitor_job_cpu=monitor_job_cpu,
            monitor_job_memory=monitor_job_memory,
            time_utility_path=time_utility_path,
            time_utility_type=time_utility_type,
        )

    def getErrorState(self):
        """
        Gets the thread-safe error state.
        """
        with self.__error_state_lock:
            return self.__error_state

    def availableSlots(self, params):
        """
        Get the number of available slots for processing jobs and
        whether or not that limit is a soft or hard limit.

        Needed so that derived schedulers can modify this limit.
        """
        if params["max_processes"] == None:
            available_slots = 1
            soft_limit = True
        else:
            available_slots = params["max_processes"]  # hard limit
            soft_limit = False
        return available_slots, soft_limit

    def triggerErrorState(self):
        with self.__error_state_lock:
            self.__error_state = True
        self.run_pool.close()
        self.status_pool.close()

    def killRemaining(self, keyboard=False):
        """Method to kill running jobs"""
        with self.__active_jobs_lock:
            for job in self.__active_jobs:
                job.killProcess()
        self.triggerErrorState()
        if keyboard:
            self.harness.keyboard_interrupt()

    def retrieveJobs(self):
        """return all the jobs the scheduler was tasked to perform work for"""
        return self.__scheduled_jobs

    def retrieveDAGs(self):
        """return all the dags containing the jobs the scueduler was tasked to perform work for"""
        return self.__dag_bank

    def schedulerError(self):
        """boolean if the scheduler prematurely exited"""
        return self.getErrorState() and not self.maxFailures()

    def maxFailures(self):
        """Boolean for hitting max failures"""
        return (
            (
                self.options.valgrind_mode
                and self.__failures >= self.options.valgrind_max_fails
            )
            or self.__failures >= self.options.max_fails
            and not self.options.hpc
        )

    def run(self, job):
        """Run a tester command"""

        # Build and set the runner that will actually run the commands
        # This is abstracted away so we can support local runners and PBS/slurm runners
        job.setRunner(self.buildRunner(job, self.options))

        tester = job.getTester()

        # Do not execute app, and do not processResults
        if self.options.dry_run:
            self.setSuccessfulMessage(tester)
            return
        # Load results from a previous run
        elif self.options.show_last_run:
            job.loadPreviousResults()
            return

        # Start job timer
        job.timer.startMain()

        # Anything that throws while running or processing a job should be caught
        # and the job should fail
        try:
            # Launch and wait for the command to finish
            job.run()

            # Set the successful message
            if not tester.isSkip() and not job.isFail():
                self.setSuccessfulMessage(tester)
        except:
            trace = traceback.format_exc()
            job.appendOutput(
                outputHeader("Python exception encountered in Job") + trace
            )
            job.setStatus(job.error, "JOB EXCEPTION")
        finally:
            # Stop job timer
            job.timer.stopMain()

    def setSuccessfulMessage(self, tester):
        """properly set a finished successful message for tester"""
        message = ""

        # Handle 'dry run' first, because if true, job.run() never took place
        if self.options.dry_run:
            message = "DRY RUN"

        elif tester.specs["check_input"]:
            message = "SYNTAX PASS"

        elif self.options.scaling and tester.specs["scale_refine"]:
            message = "SCALED"

        elif (
            self.options.enable_recover
            and tester.specs.isValid("skip_checks")
            and tester.specs["skip_checks"]
        ):
            message = "PART1"

        tester.setStatus(tester.success, message)

    def augmentJobs(self, jobs):
        """
        Allow derived schedulers to augment jobs before they perform work.
        Note: This occurs before we perform a job count sanity check. So
        any additions or subtractions to the number of jobs will result in
        an exception.
        """
        return

    def __sortAndLaunch(self):
        """
        Sort by largest DAG and launch
        """
        sorted_jobs = sorted(
            self.__dag_bank, key=lambda x: len(x[1].topological_sort()), reverse=True
        )
        for job_dag, _ in sorted_jobs:
            # Allow derived schedulers access to the jobs before they launch
            # We purposely call this one here so that we augment the Jobs
            # in the order that they're launched
            self.augmentJobs(job_dag.getJobs())
            # And launch
            self.queueJobs(job_dag)

    def setAndOutputJobStatus(self, job, status, caveats=None):
        """
        Sets a Job's status and forces the status to be output asap
        """
        job.setStatus(status)
        with job.getLock():
            job.force_report_status = True
        self.handleJobStatus(job, caveats=caveats)

    def isRunning(self):
        """
        Returns whether or not the scheduler is currently running jobs.
        """
        if self.getErrorState():
            return False
        with self.__bank_lock:
            if not self.__job_bank:
                return False
        return True

    def monitorJobProcesses(self):
        """Monitor the running job processes; called during the poll loop."""

        pass

    def getActiveJobPIDMap(self) -> dict[int, "Job"]:
        """Get the active job PID -> Job map."""
        with self.__active_jobs_lock:
            return {
                pid: job
                for job in self.__active_jobs
                if ((runner := job.getRunner()) and (pid := runner.pid))
            }

    def waitFinish(self):
        """
        Inform the Scheduler to begin running. Block until all jobs finish.
        """
        self.__sortAndLaunch()
        try:
            error_state = False

            # wait until there is an error, or job_bank has emptied
            while True:
                if not self.isRunning():
                    break
                sleep(0.1)
                self.monitorJobProcesses()

            error_state = self.getErrorState()

            # Completed all jobs sanity check
            if not error_state and self.__job_bank:
                raise SchedulerError(
                    "Scheduler exiting with different amount of work than what was initially tasked!"
                )

            if not error_state:
                self.run_pool.close()
                self.run_pool.join()
                self.status_pool.close()
                self.status_pool.join()

        except KeyboardInterrupt:
            self.killRemaining(keyboard=True)

    def getStatusPoolState(self):
        """
        Return the state of the jobs in the status TheadPool object.
        """
        # TODO: ThreadPool._state changed between python 3.7 and 3.8, this logic handles both; this
        #       logic should be changed to avoid rely on protected variables.
        (
            (self.status_pool._state != "RUN")
            if sys.version_info[1] > 7
            else self.status_pool._state
        )

    def schedule(self, testers):
        """
        Generate and submit a group of testers to a thread pool queue for execution.

        This process is serial.
        """
        # If we are not to schedule any more jobs for some reason, return now
        if self.getErrorState():
            return
        # Nothing to do if there aren't any testers
        if not testers:
            return

        # Whether or not we have parallel scheduling
        root = pyhit.load(testers[0].getSpecFile())
        parallel_scheduling = root.children[0].get("parallel_scheduling", False)

        # Instance our job DAG, create jobs, and a private lock for this group of jobs (testers)
        jobs = JobDAG(self.options, parallel_scheduling)
        j_dag = jobs.createJobs(testers)

        # job-count to tester-count sanity check
        if j_dag.size() != len(testers):
            raise SchedulerError(
                "Scheduler was going to run a different amount of testers than what was received (something bad happened)!"
            )

        # Don't need to lock below because this process is serial
        # As testers (jobs) finish, they are removed from job_bank
        self.__job_bank.update(j_dag.topological_sort())
        # List of objects relating to eachother (used for thread locking this job group)
        self.__dag_bank.append([jobs, j_dag])

        # Store all scheduled jobs
        self.__scheduled_jobs.append(j_dag.topological_sort())

    def queueJobs(self, job_dag):
        """
        Determine which queue jobs should enter. Finished jobs are placed in the status
        pool to be printed while all others are placed in the runner pool to perform work.

        A finished job will trigger a change to the Job DAG, which will allow additional
        jobs to become available and ready to enter the runner pool (dependency jobs).
        """
        state = self.getStatusPoolState()
        with job_dag.getLock():
            concurrent_jobs = job_dag.getJobsAndAdvance()
            for job in concurrent_jobs:
                if job.isFinished():
                    if not state:
                        self.handleJobStatus(job)

                elif job.isHold():
                    if not state:
                        job.setStatus(job.queued)
                        self.run_pool.apply_async(
                            self.runJob,
                            (
                                job,
                                job_dag,
                            ),
                        )

    def getLoad(self):
        """Method to return current load average"""
        loadAverage = 0.0
        try:
            loadAverage = os.getloadavg()[0]
        except AttributeError:
            pass  # getloadavg() not available in this implementation of os
        return loadAverage

    def satisfyLoad(self):
        """Method for controlling load average"""
        while self.slots_in_use > 1 and self.getLoad() >= self.average_load:
            sleep(0.1)

    def getJobSlots(self, job):
        """
        Gets the number of slots a job will use.

        This exists so that HPC runners can override it, as
        jobs like PBS jobs only use one slot because they are
        ran externally."""
        return job.getSlots()

    def reserveSlots(self, job):
        """
        Method which allocates resources to perform the job. Returns bool if job
        should be allowed to run based on available resources.
        """
        # comply with load average
        if self.options.load:
            self.satisfyLoad()

        with self.slot_lock:
            job_slots = self.getJobSlots(job)

            can_run = False
            if self.slots_in_use + job_slots <= self.available_slots:
                can_run = True

            # Check for insufficient slots -soft limit
            elif job_slots > self.available_slots and self.soft_limit:
                job.addCaveats("OVERSIZED")
                can_run = True

            # Check for insufficient slots -hard limit (skip this job)
            elif job_slots > self.available_slots and not self.soft_limit:
                job.addCaveats("insufficient slots")
                with job.getLock():
                    job.setStatus(job.skip)

            if can_run:
                self.slots_in_use += job_slots
        return can_run

    def handleTimeoutJob(self, job):
        """Handle jobs that have timed out"""
        with job.getLock():
            if job.isRunning():
                job.killProcess(job.timeout, "TIMEOUT")

    def handleJobStatus(self, job, caveats=None):
        """
        Possibly reports a job's status.

        Whether or not it actually gets reported... is not so intuitive.
        """
        # This try catch will get rid of the "Pool not running" errors
        # when we're forced to exit
        try:
            self.status_pool.apply_async(
                self.jobStatus,
                (
                    job,
                    caveats,
                ),
            )
        except ValueError:
            pass

    def jobStatus(self, job, caveats):
        """
        Instruct the TestHarness to print the status of job. This is a serial
        threaded operation, so as to prevent clobbering of text being printed
        to stdout.
        """
        # The pool is closing down due to a failure, or this job has previously been handled:
        #
        # A job which triggers the long_running timer, has a chance to finish before this
        # slower serialized status pool, has a chance to process it. Meaning two of the same
        # jobs now exist in this queue, with a finished status. This method can only work on
        # a finished job object once (a set removal operation occurs to signify scheduled job
        # completion as a sanity check).

        state = self.getStatusPoolState()
        if state:
            return
        with self.__bank_lock:
            if job not in self.__job_bank:
                return

        # Peform within a try, to allow keyboard ctrl-c
        try:
            with job.getLock():
                # This job is set to force a status
                force_status = job.force_report_status

                if force_status:
                    with self.jobs_reported_lock:
                        self.jobs_reported.add(job)
                    job.force_report_status = False
                elif job.isRunning():
                    with self.jobs_reported_lock:
                        if job in self.jobs_reported:
                            return

                    # this job will be reported as 'RUNNING'
                    if clock() - self.last_reported_time >= self.min_report_time:
                        # prevent 'finished' caveat with options expecting to take lengthy amounts of time
                        if (
                            not self.options.sep_files
                            and not self.options.hpc
                            and not self.options.heavy_tests
                            and not self.options.valgrind_mode
                        ):
                            job.addCaveats("FINISHED")

                        with self.jobs_reported_lock:
                            self.jobs_reported.add(job)
                    # TestHarness has not yet been inactive long enough to warrant a report
                    else:
                        # adjust the next report time based on delta of last report time
                        adjusted_interval = max(
                            1,
                            self.min_report_time
                            - max(1, clock() - self.last_reported_time),
                        )
                        job.report_timer = threading.Timer(
                            adjusted_interval, self.handleJobStatus, (job,)
                        )
                        job.report_timer.start()
                        return

                # Inform the TestHarness of job status
                self.harness.handleJobStatus(job, caveats=caveats)

                # Reset activity clock
                if not job.isSilent():
                    self.last_reported_time = clock()

                if job.isFail():
                    self.__failures += 1

                if job.isFinished():
                    with self.__bank_lock:
                        if job in self.__job_bank:
                            self.__job_bank.remove(job)
                        else:
                            raise SchedulerError(
                                "job accountability failure while working with: %s"
                                % (job.getTestName())
                            )

            # Max failure threshold reached, begin shutdown
            if self.maxFailures():
                self.killRemaining()

        except Exception:
            print("statusWorker Exception: %s" % (traceback.format_exc()))
            self.killRemaining()

        except KeyboardInterrupt:
            self.killRemaining(keyboard=True)

    def runJob(self, job, jobs):
        """Method the run_pool calls when an available thread becomes ready"""
        # Its possible, the queue is just trying to empty. Allow it to do so
        # with out generating overhead
        if self.getErrorState():
            return

        try:
            # see if we have enough slots to start this job
            if self.reserveSlots(job):
                with job.getLock():
                    job.setStatus(job.running)

                    # Setup the long running timer, if any
                    if self.report_long_jobs:
                        job.report_timer = threading.Timer(
                            self.min_report_time, self.handleJobStatus, (job,)
                        )
                        job.report_timer.start()
                    else:
                        job.report_timer = None

                with self.__active_jobs_lock:
                    self.__active_jobs.add(job)

                if self.enforce_timeout:
                    timeout_timer = threading.Timer(
                        float(job.getMaxTime()), self.handleTimeoutJob, (job,)
                    )
                    timeout_timer.start()
                else:
                    timeout_timer = None

                # We have a try here because we want to explicitly catch things like
                # python errors in _only_ the Job; exceptions that happen in the Tester
                # from within the Job will get caught within the Tester
                try:
                    self.run(job)  # Hand execution over to derived scheduler
                except Exception:
                    with job.getLock():
                        trace = traceback.format_exc()
                        job.setStatus(StatusSystem().error, "JOB RUN EXCEPTION")
                        job.appendOutput(
                            f"Encountered an exception while running Job:\n{trace}"
                        )

                if timeout_timer:
                    timeout_timer.cancel()

                # Recover worker count before attempting to queue more jobs
                with self.slot_lock:
                    self.slots_in_use = max(
                        0, self.slots_in_use - self.getJobSlots(job)
                    )

                with job.getLock():
                    # Stop the long running timer
                    if job.report_timer:
                        job.report_timer.cancel()

                    # All done
                    job.setStatus(StatusSystem().finished)

                with self.__active_jobs_lock:
                    self.__active_jobs.remove(job)

            # Not enough slots to run the job...
            else:
                # ...currently, place back on hold before placing it back into the queue
                if not job.isFinished():
                    with job.getLock():
                        job.setStatus(job.hold)

            # Job is done (or needs to re-enter the queue)
            self.queueJobs(jobs)

        except Exception:
            print("runWorker Exception: %s" % (traceback.format_exc()))
            self.killRemaining()

        except KeyboardInterrupt:
            self.killRemaining(keyboard=True)

    def appendResultFooter(self, stats: dict) -> str:
        """Entrypoint to add additional results to the on screen result footer"""
        return None

    def appendResultFileHeader(self) -> dict:
        """Entrypoint to add entries to the result file header"""
        return {}

    def appendResultFileJob(self, job) -> dict:
        """Entrypoint to add entries to the result file for a job"""
        return {}

    def appendStats(self) -> dict:
        """Entrypoint to add entries to the harness statistics"""
        return {}
