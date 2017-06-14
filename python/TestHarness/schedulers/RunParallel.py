from subprocess import *
from time import sleep
from timeit import default_timer as clock

from tempfile import TemporaryFile
from Tester import Tester
from signal import SIGTERM
from Scheduler import Scheduler
from MooseObject import MooseObject
import platform

import os

## This class provides an interface to run commands in parallel
#
# To use this class, call the .run() method with the command and the test
# options. When the test is finished running it will call harness.testOutputAndFinish
# to complete the test. Be sure to call join() to make sure all the tests are finished.
#
class RunParallel(Scheduler):
    @staticmethod
    def validParams():
        params = Scheduler.validParams()
        params.addRequiredParam('scheduler',    'RunParallel', "The type of scheduler.")

        return params

    ## Return this return code if the process must be killed because of timeout
    TIMEOUT = -999999

    def __init__(self, harness, params):
        Scheduler.__init__(self, harness, params)

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
                tester.setStatus('Insufficient slots', tester.bucket_skip)
                self.harness.handleTestStatus(tester)
                self.skipped_jobs.add(tester.specs['test_name'])
            return

        # Now make sure that this job doesn't have an unsatisfied prereq
        if self.unsatisfiedPrereqs(tester):
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
        tester.prepare(self.options)

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

    def checkOutputReady(self, tester):
        # See if the output is available, it'll be available if we are just using the TemporaryFile mechanism
        # If we are redirecting output though, it might take a few moments for the file buffers to appear
        # after one of the process quits.
        if not tester.specs.isValid('redirect_output') or not tester.specs['redirect_output'] or tester.getProcs(self.options) == 1:
            return True

        for processor_id in xrange(tester.getProcs(self.options)):
            # Obtain path and append processor id to redirect_output filename
            file_path = os.path.join(tester.specs['test_dir'], tester.name() + '.processor.{}'.format(processor_id))
            if not os.access(file_path, os.R_OK):
                return False
        return True

    def getOutputFromFiles(self, tester):
        file_output = ''
        for processor_id in xrange(tester.getProcs(self.options)):
            # Obtain path and append processor id to redirect_output filename
            file_path = os.path.join(tester.specs['test_dir'], tester.name() + '.processor.{}'.format(processor_id))
            if os.access(file_path, os.R_OK):
                with open(file_path, 'r') as f:
                    file_output += "#"*80 + "\nOutput from processor " + str(processor_id) + "\n" + "#"*80 + "\n" + self.readOutput(f)
        return file_output

    ## Return control to the test harness by finalizing the test output and calling the callback
    def returnToTestHarness(self, job_index):
        (p, command, tester, time, f, slots) = self.jobs[job_index]

        log( 'Command %d done:    %s' % (job_index, command) )
        output = 'Working Directory: ' + tester.specs['test_dir'] + '\nRunning command: ' + command + '\n'

        # See if there's already a fail status set on this test. If there is, we shouldn't attempt to read from the files
        # Note: We cannot use the didPass() method on the tester here because the tester hasn't had a chance to set
        # status yet in the postprocessing stage. We'll inspect didPass() after processing results
        if not tester.didFail():
            # Read the output either from the temporary file or redirected files
            if tester.specs.isValid('redirect_output') and tester.specs['redirect_output'] and tester.getProcs(self.options) > 1:
                # If the tester enabled redirect_stdout and is using more than one processor
                redirected_output = self.getOutputFromFiles(tester)
                output += redirected_output

                # If we asked for redirected output but none was found, we'll call that a failure
                if redirected_output == '':
                    tester.setStatus('FILE TIMEOUT', tester.bucket_fail)
                    output += '\n' + "#"*80 + '\nTester failed, reason: ' + tester.getStatusMessage() + '\n'
            else:
                # Handle the case were the tester did not inherite from RunApp (like analyzejacobian)
                output += self.readOutput(f)
        else:
            output += '\n' + "#"*80 + '\nTester failed, reason: ' + tester.getStatusMessage() + '\n'

        if p.poll() == None: # process has not completed, it timed out
            output += '\n' + "#"*80 + '\nProcess terminated by test harness. Max time exceeded (' + str(tester.specs['max_time']) + ' seconds)\n' + "#"*80 + '\n'
            f.close()
            tester.setStatus('TIMEOUT', tester.bucket_fail)
            if platform.system() == "Windows":
                p.terminate()
            else:
                pgid = os.getpgid(p.pid)
                os.killpg(pgid, SIGTERM)

            if self.options.pbs and not self.options.processingPBS:
                self.harness.handleTestStatus(tester, output)
            else:
                self.harness.testOutputAndFinish(tester, RunParallel.TIMEOUT, output, time, clock())
        else:
            f.close()

            # PBS jobs
            if self.options.pbs and not self.options.processingPBS:
                self.harness.handleTestStatus(tester, output)

            # All other jobs
            else:
                if tester in self.reported_jobs:
                    tester.specs.addParam('caveats', ['FINISHED'], "")

                if self.options.dry_run:
                    # Set the successful message for DRY_RUN
                    tester.success_message = 'DRY RUN'
                    output += '\n'.join(tester.processResultsCommand(tester.specs['moose_dir'], self.options))
                elif tester.getStatus() != 'FAIL': # Still haven't processed results!
                    output = tester.processResults(tester.specs['moose_dir'], p.returncode, self.options, output)

                self.harness.testOutputAndFinish(tester, p.returncode, output, time, clock())

        if tester.didPass():
            self.finished_jobs.add(tester.specs['test_name'])
        else:
            self.skipped_jobs.add(tester.specs['test_name'])

        self.jobs[job_index] = None
        self.slots_in_use = self.slots_in_use - slots

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
