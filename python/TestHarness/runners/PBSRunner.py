#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarness.runners.Runner import Runner
import time, os

class PBSRunner(Runner):
    """Runner that spawns a process with PBS.

    To be used with the RunPBS scheduler.
    """
    def __init__(self, job, options, run_pbs):
        Runner.__init__(self, job, options)
        self.run_pbs = run_pbs

        # Number of seconds to try to wait for the output
        # We don't want to wait forever for output because
        # if the job ended in an unexpected state, it might
        # not even be using the output and we don't want to
        # just hang forever
        self.wait_output_time = 60

    def spawn(self, timer):
        from TestHarness.schedulers.RunPBS import RunPBS

        # Submit the job
        self.run_pbs.submitJob(self.job)

        timer.start()

    def wait(self, timer):
        # Need to import here to avoid cyclic includes
        from TestHarness.schedulers.RunPBS import RunPBS

        # Poll loop waiting for the job to be finished
        # This gets a structure that represents the job, and the
        # polling itself is only done on occasion within RunPBS
        while True:
            time.sleep(1)
            pbs_job = self.run_pbs.getPBSJob(self.job)

            # We're done
            if pbs_job.done:
                self.exit_code = pbs_job.exit_code
                break

        timer.stop()

        # The PBS output (stdout+stderr)
        output_file = self.run_pbs.getPBSJobOutputPath(self.job)

        # If the Job is already finished, something happened in PBS
        # so we have an invalid state for processing in the Tester
        if self.job.isFinished():
            self.exit_code = -1
            self.output = ''

            # If we have output, we should try to add it
            # TODO: shorten output as an option?
            if os.path.exists(output_file) and os.path.isfile(output_file):
                try:
                    self.output = open(file, 'r').read()
                except:
                    pass

            # Don't bother looking for the rest of the output
            return

        tester = self.job.getTester()

        # We've actually ran something now and not just qsub, so update the
        # command to what was ran there
        tester.setCommandRan(pbs_job.command)

        # Determine the output files that we need to wait for to be complete
        wait_files = set([output_file])
        # Output files needed by the Tester, only if it says we should
        if tester.mustOutputExist():
            for file in tester.getOutputFiles(self.options):
                wait_files.add(os.path.join(tester.getTestDir(), file))

        # Wait for all of the files to be available
        file_poll_interval = 0.5
        waited_time = 0
        while wait_files:
            # Look for each file
            for file in wait_files.copy():
                # File exists
                if os.path.exists(file) and os.path.isfile(file):
                    # Special case for stdout/stderr, where we append
                    # something to the end to show that it's actually done
                    # and then need to read it
                    # TODO: shorten output as an option?
                    if file == output_file:
                        output = open(file, 'r').read()
                        ending_comment = self.run_pbs.getOutputEndingComment()
                        if ending_comment in output:
                            self.output = output.replace(ending_comment, '')
                        else:
                            continue
                    # Done with this file
                    wait_files.discard(file)

            # We've waited for files for too long
            if wait_files and waited_time >= self.wait_output_time:
                self.job.setStatus(self.job.error, 'FILE TIMEOUT')
                if not self.output:
                    self.output = ''
                self.output += '#' * 80 + '\nUnavailable output file(s)\n' + '#' * 80 + '\n'
                for file in wait_files:
                    self.output += file + '\n'
                self.output += '\n'
                break

            waited_time += file_poll_interval
            time.sleep(file_poll_interval)

    def kill(self):
        self.run_pbs.killJob(self.job)
