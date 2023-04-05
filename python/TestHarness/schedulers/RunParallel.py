#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import traceback

from TestHarness.schedulers.Scheduler import Scheduler
from TestHarness.StatusSystem import StatusSystem
from TestHarness import util

class RunParallel(Scheduler):
    """
    RunParallel is a Scheduler plugin responsible for executing a tester
    command and doing something with its output.
    """
    @staticmethod
    def validParams():
        params = Scheduler.validParams()
        return params

    def __init__(self, harness, params):
        Scheduler.__init__(self, harness, params)

    def run(self, job):
        """ Run a tester command """

        tester = job.getTester()

        # Do not execute app, and do not processResults
        if self.options.dry_run:
            self.setSuccessfulMessage(tester)
            return
        elif self.options.show_last_run:
            job_results = self.options.results_storage[job.getTestDir()][job.getTestName()]
            status, message, caveats = job.previousTesterStatus(self.options, self.options.results_storage)
            tester.setStatus(status, message)
            if caveats:
                tester.addCaveats(caveats)
            job.setPreviousTime(job_results['TIMING'])
            job.setOutput(job_results['OUTPUT'])
            return

        output = ''

        # Anything that throws while running or processing a job should be caught
        # and the job should fail
        try:
            # Launch and wait for the command to finish
            job.run()

            # Was this job already considered finished? (Timeout, Crash, etc)
            if job.isFinished():
                return

            # Allow derived proccessResults to process the output and set a failing status (if it failed)
            job_output = job.getOutput()
            output = tester.processResults(tester.getMooseDir(), self.options, job_output)

            # If the tester requested to be skipped at the last minute, report that.
            if tester.isSkip():
                output += '\n' + "#"*80 + '\nTester skipped, reason: ' + tester.getStatusMessage() + '\n'
            elif tester.isFail():
                output += '\n' + "#"*80 + '\nTester failed, reason: ' + tester.getStatusMessage() + '\n'
            # If the tester has not yet failed, append additional information to output
            else:
                # Read the output either from the temporary file or redirected files
                if tester.hasRedirectedOutput(self.options):
                    redirected_output = util.getOutputFromFiles(tester, self.options)
                    output += redirected_output

                    # If we asked for redirected output but none was found, we'll call that a failure
                    if redirected_output == '':
                        tester.setStatus(tester.fail, 'FILE TIMEOUT')
                        output += '\n' + "#"*80 + '\nTester failed, reason: ' + tester.getStatusMessage() + '\n'

                self.setSuccessfulMessage(tester)
        except Exception as e:
            output += 'Python exception encountered:\n\n' + traceback.format_exc()
            tester.setStatus(StatusSystem().error, 'TESTER EXCEPTION')

        if job.getOutputFile():
            job.addMetaData(DIRTY_FILES=[job.getOutputFile()])

        # Set testers output with modifications made above so it prints the way we want it
        job.setOutput(output)

    def setSuccessfulMessage(self, tester):
        """ properly set a finished successful message for tester """
        message = ''

        # Handle 'dry run' first, because if true, job.run() never took place
        if self.options.dry_run:
            message = 'DRY RUN'

        elif tester.specs['check_input']:
            message = 'SYNTAX PASS'

        elif self.options.scaling and tester.specs['scale_refine']:
            message = 'SCALED'

        elif self.options.enable_recover and tester.specs.isValid('skip_checks') and tester.specs['skip_checks']:
            message = 'PART1'

        tester.setStatus(tester.success, message)
