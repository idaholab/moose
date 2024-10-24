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
from TestHarness import util
from TestHarness.runners.SubprocessRunner import Runner, SubprocessRunner
from TestHarness.testers.Tester import Tester

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
            job.appendOutput(util.outputHeader('Python exception encountered in Job') + trace)
            job.setStatus(job.error, 'JOB EXCEPTION')

    def buildRunner(self, job, options) -> Runner:
        """Builds the runner for a given tester

        This exists as a method so that derived schedulers can change how they
        run commands (i.e., for PBS and slurm)
        """
        return SubprocessRunner(job, options)

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
