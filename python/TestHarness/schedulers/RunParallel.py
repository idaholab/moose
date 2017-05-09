from timeit import default_timer as clock

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
        params.addRequiredParam('scheduler',    'RunParallel', "The name of this scheduler")

        return params

    def __init__(self, harness, params):
        Scheduler.__init__(self, harness, params)

    def getTests(self):
        print 'Not supported'
        return []

    ## run the command asynchronously and call testharness.testOutputAndFinish when complete
    def run(self, tester, command, recurse=True, slot_check=True):
        # First see if any of the queued jobs can be run but only if recursion is allowed on this run
        if recurse:
            self.startReadyJobs(slot_check)

        # Now make sure that this job doesn't have an unsatisfied prereq
        if self.unsatisfiedPrereqs(tester):
            self.queue.append([tester, command, os.getcwd()])
            return

        # We're good up to this point? Then launch the job!
        self.launchJob(tester, command, slot_check)

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
    def returnToTestHarness(self, job_index, output):
        (p, command, tester, time, f, slots) = self.jobs[job_index]

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

        if tester in self.reported_jobs:
            tester.specs.addParam('caveats', ['FINISHED'], "")

        if tester.getStatus() != 'FAIL': # Still haven't processed results!
            output = tester.processResults(tester.specs['moose_dir'], p.returncode, self.options, output)

        self.harness.testOutputAndFinish(tester, p.returncode, output, time, clock())

        if tester.didPass():
            self.finished_jobs.add(tester.specs['test_name'])
        else:
            self.skipped_jobs.add(tester.specs['test_name'])
