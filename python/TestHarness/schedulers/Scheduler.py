from time import sleep
from timeit import default_timer as clock
from tempfile import TemporaryFile
from collections import deque
from MooseObject import MooseObject
from Queue import Queue
from signal import SIGTERM
from util import *
import os, sys, platform, subprocess

from multiprocessing.pool import ThreadPool

class Scheduler(MooseObject):
    """
    Base class for handling how jobs are launched

    To use this class, call .schedule() and supply a list of testers to schedule

    Syntax:
       Scheduler.schedule([list of tester objects])

    Those testers will be added to a queue. You can continue to add testers to the queue
    in this fashion.

    Once you schedule all the testers you wish to test, call .join() to begin executing them.
    As tests complete (or are skipped), the Scheduler will call the call back the harness.testOutputAndFinish() method
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

        # Initialize runner pool based on available slots
        self.runner_pool = ThreadPool(processes=self.job_slots)

        # Initialize status pool to only use 1 process (to prevent status messages from getting clobbered)
        self.status_pool = ThreadPool(processes=1)

        # Set the time threshold when to report long running jobs
        self.default_reported_time = 10.0

        # Initialize Scheduler queue objects
        self.clearAndInitializeJobs()

    ## Clear and Initialize the scheduler queue
    def clearAndInitializeJobs(self):
        # A set holding tests that were skipped
        self.skipped_jobs = set([])

        # A set holding tests that were finished
        self.finished_jobs = set([])

        # A set holding tests that were scheduled
        self.scheduled_jobs = set([])

        # Current slots in use
        self.workers_in_use = 0

        # Runner Queue (we put jobs we want to run asyncronously in this)
        # runner_queue.put( (tester) )
        self.runner_queue = Queue()

        # Status queue (we put jobs we want to display a status for in this)
        # status_queue.put( (tester, clock(), subprocess_object) )
        self.status_queue = Queue()

    # Return post run command from derived classes
    def postCommand(self):
        return

    # Allow derived schedulers to initiate findAndRunTests again
    def goAgain(self):
        return

    ## Run the command asynchronously
    def run(self, tester, command):
        (tester, output_file, start_time) = self.launchJob(tester, command)
        return (tester, output_file, start_time)

    ## process test results
    def returnToTestHarness(self, tester):
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

    # return True if this test has unsatified prereqs
    def unsatisfiedPrereqs(self, tester):
        if tester.specs['prereq'] != None and len(set(tester.specs['prereq']) - self.finished_jobs):
            if self.options.pbs is None and self.options.ignored_caveats is None:
                return True
            else:
                if self.options.ignored_caveats:
                    caveat_list = [x.lower() for x in self.options.ignored_caveats.split()]
                    if 'all' not in self.options.ignored_caveats and 'prereq' not in self.options.ignored_caveats:
                        return True
        return False

    # Allow derived schedulers to skip tests
    def canLaunch(self, tester, checks, test_list):
        return tester.checkRunnableBase(self.options, checks, test_list)

    # Loop through the testers and add them to the runner pool for execution
    def schedule(self, testers, checks=None, test_list=None):

        self.checks = checks
        self.test_list = test_list

        # generate an ordered list of tests so we launch them in the correct order
        forward_set = self.getDependencies(testers)

        # loop through the list of sets of tests
        for test_set in forward_set:
            for tester in test_set:
                self.scheduled_jobs.add(tester.getTestName())
                self.runner_queue.put(tester)
                results = self.runner_pool.apply_async(self.debug_run, (self.runner_queue, checks, test_list,))
                #print results.get()

    # Launch jobs stored in our queues. Method is blocked until queue(s) are empty
    def waitFinish(self):
        while not self.status_queue.empty() or not self.runner_queue.empty():

            # Assign a job to handle jobs placed in the status queue
            if not self.status_queue.empty():
                results = self.status_pool.apply_async(self.debug_status, (self.status_queue,))

            # A runner job was placed back in the queue due to unsatisfied prereqs
            if not self.runner_queue.empty():
                results = self.runner_pool.apply_async(self.debug_run, (self.runner_queue, self.checks, self.test_list,))

            # Sleep for a little bit
            sleep(0.01)

    # return available worker count
    def workersAvailable(self):
        return self.job_slots - self.workers_in_use

    # return tester object from test name
    def getTesterByName(self, testers, test_name):
        for tester in testers:
            if test_name == tester.getTestName():
                return tester

    # return list of sets of tests
    def getDependencies(self, testers):
        f = DependencyResolver()
        for tester in testers:
            dependency_objects = set([])
            if tester.getPrereqs() != []:
                for test in tester.getPrereqs():
                    dependency_objects.add(self.getTesterByName(testers, test))

            f.insertDependency(tester, dependency_objects)

        return f.getSortedValuesSets()

    # Execute a command and return the process, output_file and time the process was launched
    def launchJob(self, tester, command):
        # It seems that using PIPE doesn't work very well when launching multiple jobs.
        # It deadlocks rather easy.  Instead we will use temporary files
        # to hold the output as it is produced
        try:
            f = TemporaryFile()
            # On Windows, there is an issue with path translation when the command is passed in
            # as a list.
            if platform.system() == "Windows":
                process = subprocess.Popen(command,stdout=f,stderr=f,close_fds=False, shell=True, creationflags=CREATE_NEW_PROCESS_GROUP, cwd=tester.getTestDir())
            else:
                process = subprocess.Popen(command,stdout=f,stderr=f,close_fds=False, shell=True, preexec_fn=os.setsid, cwd=tester.getTestDir())
        except:
            print "ERROR TESTER:", tester.getTestName()
            print "Error in launching a new task", command
            raise

        return (process, f, clock())

    # debug status processing
    def debug_status(self, queue):
        (tester, last_checked_time, process) = queue.get()

        # Test is still running
        if tester.isPending():
            test_start_time = tester.getStartTime()
            now = clock()

            # Check if this test is timing out
            if now - test_start_time > tester.getMaxTime():
                print 'TIMEOUT', tester.getTestName()
                tester.setStatus('TIMEOUT', tester.bucket_fail)
                if process is not None:
                    process.kill()
                return

            # Test has been running long enough we should inform the user about it
            elif now - last_checked_time >= self.default_reported_time:
                tester.setStatus('RUNNING...', tester.bucket_pending)

                # Inform the TestHarness of a status change
                self.harness.handleTestStatus(tester)

            # Place this test back in the queue
            queue.put((tester, clock(), process))
            return

        # Test is ready to have its results printed
        else:
            # Note: add_to_table=True tells the TestHarness this is a finished test
            self.harness.handleTestStatus(tester, add_to_table=True)

        if tester.didPass():
            self.finished_jobs.add(tester.getTestName())
        else:
            self.skipped_jobs.add(tester.getTestName())

    # debug runner processing
    def debug_run(self, queue, checks, test_list):
        # Grab a job from the queue (blocking)
        tester = queue.get()

        if self.canLaunch(tester, checks, test_list):
            # If this test is allowed to run, but has unsatisfied prereqs, place it back in the queue
            if self.unsatisfiedPrereqs(tester):
                for prereq in tester.getPrereqs():

                    # This test requires a prereq that was never scheduled
                    if prereq not in self.scheduled_jobs:
                        tester.setStatus('skipped dependency', tester.bucket_skip)
                        self.status_queue.put((tester, clock(), None))
                        return

                queue.put(tester)
                return

            # Inform the scheduler we are using x slots
            self.workers_in_use += tester.getProcs(self.options)

            # We now have a job. Get the command to run
            command = tester.getCommand(self.options)

            # Launch the command and start the clock
            (process, output_file, start_time) = self.run(tester, command)
            tester.start_time = start_time

            # Inform the status queue there is a job to check status on. This allows the status_worker
            # pool to assign a thread to check the status of a test as its running (allows for long
            # running tests to print RUNNING...)
            self.status_queue.put((tester, start_time, process))

            # Wait for the process to complete
            process.wait()

            if process.poll() is not None:
                tester.exit_code = process.poll()
                tester.std_outfile = output_file
                stuff = self.returnToTestHarness(tester)

            # This test failed to launch properly
            else:
                tester.setStatus('ERROR LAUNCHING JOB', tester.bucket_fail)

                # Allow status pool thread to print these results
                self.status_queue.put((tester, clock(), None))
                self.status_pool.apply_async(self.debug_status, (self.status_queue,))

            # Close the temp file
            output_file.close()

            # Relinquish the slots we were using
            self.workers_in_use = max(0, self.workers_in_use - tester.getProcs(self.options))

        # This job is skipped, deleted, or silent
        else:
            self.skipped_jobs.add(tester.getTestName())
            self.scheduled_jobs.remove(tester.getTestName())

            # Allow status pool thread to print these results
            self.status_queue.put((tester, clock(), None))
            self.status_pool.apply_async(self.debug_status, (self.status_queue,))
