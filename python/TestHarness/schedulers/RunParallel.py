#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarness.schedulers.Scheduler import Scheduler
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

        # Launch and wait for the command to finish
        job.run()

        # Was this job already considered finished? (Timeout, Crash, etc)
        if job.isFinished():
            return

        # If we are doing recover tests
        if self.options.enable_recover and tester.specs.isValid('skip_checks') and tester.specs['skip_checks']:
            tester.setStatus(tester.success, 'PART1')
            return

        # Allow derived proccessResults to process the output and set a status
        job_output = job.getOutput()
        output = tester.processResults(tester.getMooseDir(), self.options, job_output)

        # See if there's already a failing status set on this test. If there is, we shouldn't attempt to
        # read from the redirected output files.
        if not tester.isFail():
            # Read the output either from the temporary file or redirected files
            if tester.hasRedirectedOutput(self.options):
                redirected_output = util.getOutputFromFiles(tester, self.options)
                output += redirected_output

                # If we asked for redirected output but none was found, we'll call that a failure
                if redirected_output == '':
                    tester.setStatus(tester.fail, 'FILE TIMEOUT')
                    output += '\n' + "#"*80 + '\nTester failed, reason: ' + tester.getStatusMessage() + '\n'

        else:
            output += '\n' + "#"*80 + '\nTester failed, reason: ' + tester.getStatusMessage() + '\n'

        # Set testers output with modifications made above so it prints the way we want it
        job.setOutput(output)
