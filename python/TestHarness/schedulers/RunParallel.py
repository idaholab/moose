from timeit import default_timer as clock

from Tester import Tester
from Scheduler import Scheduler
from MooseObject import MooseObject
import util
import platform, time

import os

## This class provides an interface to run tester commands and do something with the output
#
# To use this class, call the .run() method with the tester_data container
class RunParallel(Scheduler):
    @staticmethod
    def validParams():
        params = Scheduler.validParams()
        return params

    def __init__(self, harness, params):
        Scheduler.__init__(self, harness, params)

    ## Run the test!
    ## Note: use thread_lock where necessary
    #
    #  with self.thread_lock:
    #      do protected stuff
    #
    # The run method should be blocking until the test has completed _and_ the results have been
    # processed (with tester.processResults()). When we return from this method, this test will
    # be considered finished, the output file will be closed, and a worker will immediately place
    # this job in the status queue, to have its status printed to the screen.
    def run(self, job_container):
        tester = job_container.getTester()

        # Get the command needed to run this test
        command = tester.getCommand(self.options)

        # Prepare to run the test
        tester.prepare(self.options)

        # Launch and wait for the command to finish
        process = job_container.runCommand(command)

        # Was this test already considered finished? (Timeouts, Dry Run)
        if tester.isFinished():
            return

        if process.poll() is not None:
            # get the output for this test
            output = job_container.getOutput()

            # If we are doing recover tests
            if self.options.enable_recover and tester.specs.isValid('skip_checks') and tester.specs['skip_checks']:
                tester.setStatus('PART1', tester.bucket_success)
                return
            else:
                # Process the results and beautify the output
                self.testOutput(job_container, output)

        # This test failed to launch properly
        else:
            tester.setStatus('ERROR LAUNCHING JOB', tester.bucket_fail)

    # Modify the output the way we want it. Run processResults
    def testOutput(self, job_container, output):
        tester = job_container.getTester()

        # process and store new results from output
        output = tester.processResults(tester.getMooseDir(), job_container.getExitCode(), self.options, output)

        # See if there's already a fail status set on this test. If there is, we shouldn't attempt to
        # read from the files

        # Note: We cannot use the didPass() method on the tester here because the tester
        # hasn't had a chance to set status yet in the postprocessing stage. We'll inspect didPass() after
        # processing results
        if not tester.didFail():
            # Read the output either from the temporary file or redirected files
            if tester.hasRedirectedOutput(self.options):
                redirected_output = util.getOutputFromFiles(tester, self.options)
                output += redirected_output

                # If we asked for redirected output but none was found, we'll call that a failure
                if redirected_output == '':
                    tester.setStatus('FILE TIMEOUT', tester.bucket_fail)
                    output += '\n' + "#"*80 + '\nTester failed, reason: ' + tester.getStatusMessage() + '\n'

        else:
            output += '\n' + "#"*80 + '\nTester failed, reason: ' + tester.getStatusMessage() + '\n'

        # Set testers output with modifications made above so it prints the way we want it
        job_container.setOutput(output)
