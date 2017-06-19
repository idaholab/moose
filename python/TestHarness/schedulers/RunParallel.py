from timeit import default_timer as clock

from Tester import Tester
from Scheduler import Scheduler
from MooseObject import MooseObject
from util import *
import platform, time

import os

## This class provides an interface to run commands in parallel
#
# To use this class, call the .run() method with the tester options
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
    # The run method should be blocking until the test has completed _and_ the results have been
    # processed (processResults()). When we return from this method, this test will be considered
    # finished and added to the status pool for printing out the final results.
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
                        self.testOutput(tester, '', options, thread_lock)
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

                    # set the tester output (trimmed)
                    results = readOutput(output_file, options)
                    output_file.close()

                    # Process the results and beautify the output
                    self.testOutput(tester, results, options, thread_lock)

                # This test failed to launch properly
                else:
                    tester.setStatus('ERROR LAUNCHING JOB', tester.bucket_fail)

    # Modify the output the way we want it. Run processResults
    def testOutput(self, tester, results, options, thread_lock):
        # create verbose header
        output = 'Working Directory: ' + tester.getTestDir() + '\nRunning command: ' + tester.getCommand(options) + '\n'

        # dry run? cool, just return
        if options.dry_run or not tester.shouldExecute():
            tester.std_out = output
            tester.setStatus(tester.getSuccessMessage(), tester.bucket_success)
            return

        # OKAY! so, here is where we need a thread_lock :). Some of the restart tests collide during tester.prepare and this method.
        # processResults is _very_ expensive. Enough to warrant a check if we need to lock it (32 cores Locked: 43sec, Not locked: 27sec)
        if tester.getPrereqs():
            with thread_lock:
                # process and append the test results to output
                output += tester.processResults(tester.getMooseDir(), tester.getExitCode(), options, results)
        else:
            # process and append the test results to output
            output += tester.processResults(tester.getMooseDir(), tester.getExitCode(), options, results)

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

        # Set testers output with modifications made above
        tester.std_out = output
