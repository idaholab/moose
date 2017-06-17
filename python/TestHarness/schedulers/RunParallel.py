from timeit import default_timer as clock

from Tester import Tester
from Scheduler import Scheduler
from MooseObject import MooseObject
from util import *
import platform, time

import os

## This class provides an interface to run commands in parallel
#
# To use this class, call the .run() method with the tester
# options. When the test is finished running it will call returnToTestHarness
class RunParallel(Scheduler):
    @staticmethod
    def validParams():
        params = Scheduler.validParams()
        params.addRequiredParam('scheduler',    'RunParallel', "The name of this scheduler")

        return params

    def __init__(self, harness, params):
        Scheduler.__init__(self, harness, params)

    ## Run the test!
    ## Note: use thread_lock when writing changes to the tester object
    #
    #  with thread_lock:
    #      do protected stuff
    #
    # The run method should be blocking. When we return from this method, this test should be considered
    # finished as we will lose our thread running this job. Apon which a new single processing thread will
    # add our finished job to a new queue, to print the results (harness.handleTestStatus)
    def run(self, tester, testers, thread_lock, options):
        # Ask the tester if its allowed to run on this machine
        if tester.checkRunnableBase(options):

            # Ask if this test is allowed to run based on other test's circumstances
            status_check = StatusDependency(tester, testers, options)
            if status_check.checkAndSetStatus():
                with thread_lock:
                    # Get the command needed to run this test
                    command = tester.getCommand(options)

                    # Prepare to run the test
                    tester.prepare(options)

                    # If shouldExecute was false, there is process or output to work with so pass
                    # control over to testOutput now, to run processResults and complete the test
                    if not tester.shouldExecute() or options.dry_run:
                        tester.start_time = clock()
                        tester.exit_code = 0
                        tester.end_time = clock()
                        tester.std_out = ''
                        self.testOutput(tester, options, thread_lock)
                        return
                    else:
                        # Launch the command and start the clock
                        (process, output_file, start_time) = returnCommand(tester, command)

                    # Set the tester's start time
                    tester.start_time = start_time

                # We're a thread so wait for the process to complete outside the thread_lock
                process.wait()
                tester.end_time = clock()

                # Did the process fail and we didn't know it (timeouts)?
                if tester.didFail():
                    return

                if process.poll() is not None:
                    # set the exit code
                    tester.exit_code = process.poll()

                    # set the output (trimmed)
                    tester.std_out = readOutput(output_file, options)

                    # Process the results and beautify the output
                    output = self.testOutput(tester, options, thread_lock)
                    output_file.close()

                # This test failed to launch properly
                else:
                    tester.setStatus('ERROR LAUNCHING JOB', tester.bucket_fail)

            # This test can not run due to statuses from the other testers
            else:
                # No need to set any statuses. The appropriate status has already been set
                return
        # This job is skipped, deleted, or silent as deemed by the tester, but after it was scheduled
        else:
            # No need to set any statuses. checkRunnable does that for us, so just return
            return

    # Modify the output the way we want it. Run processResults
    def testOutput(self, tester, options, thread_lock):
        # create verbose header
        output = 'Working Directory: ' + tester.getTestDir() + '\nRunning command: ' + tester.getCommand(options) + '\n'

        # dry run? cool, just return
        if options.dry_run:
            tester.setOutput(output)
            tester.setStatus(tester.getSuccessMessage(), tester.bucket_success)
            return

        # OKAY! so, here is where we need a thread_lock :). Some of the restart tests collide during tester.prepare and this method.
        # processResults is _very_ expensive. Enough to warrant a check if we need to lock it (32 cores Locked: 43sec, Not locked: 27sec)
        if tester.getPrereqs():
            with thread_lock:
                # process and append the test results to output
                output += tester.processResults(tester.getMooseDir(), tester.getExitCode(), options, tester.getOutput())
        else:
            # process and append the test results to output
            output += tester.processResults(tester.getMooseDir(), tester.getExitCode(), options, tester.getOutput())

        # See if there's already a fail status set on this test. If there is, we shouldn't attempt to read from the files
        # Note: We cannot use the didPass() method on the tester here because the tester hasn't had a chance to set
        # status yet in the postprocessing stage. We'll inspect didPass() after processing results
        if not tester.didFail():
            # Read the output either from the temporary file or redirected files
            if tester.hasRedirectedOutput(options):
                redirected_output = getOutputFromFiles(tester, options)
                output += redirected_output

                # If we asked for redirected output but none was found, we'll call that a failure
                if redirected_output == '':
                    tester.setStatus('FILE TIMEOUT', tester.bucket_fail)
                    output += '\n' + "#"*80 + '\nTester failed, reason: ' + tester.getStatusMessage() + '\n'

        else:
            output += '\n' + "#"*80 + '\nTester failed, reason: ' + tester.getStatusMessage() + '\n'

        # Adjust testers output with modifications made above
        tester.setOutput(output)
