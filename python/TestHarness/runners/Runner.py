#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import json

class Runner:
    """
    Base class for running a process via a command.

    Used within the Tester to actually run a test. We need
    this specialized so that we can either run things locally
    or externally (i.e., PBS, slurm, etc on HPC)
    """
    def __init__(self, job, options):
        # The job that this runner is for
        self.job = job
        # The test harness options
        self.options = options
        # The job's exit code, should be set after wait()
        self.exit_code = None
        # The output the job produced; to be filled in wait()
        self.output = ''

    def spawn(self, timer):
        """
        Spawns the process.

        Wait for the process to complete with wait().

        Should be overridden.
        """
        pass

    def wait(self, timer):
        """
        Waits for the process started with spawn() to complete.

        Should be overridden.
        """
        pass

    def kill(self):
        """
        Kills the process started with spawn()

        Should be overridden.
        """
        pass

    def getOutput(self):
        """
        Gets the combined output of the process.
        """
        output = self.output

        # Check for invalid unicode in output
        try:
            json.dumps(output)
        except UnicodeDecodeError:
            # Convert invalid output to something json can handle
            output = output.decode('utf-8','replace').encode('ascii', 'replace')
            # Alert the user that output has invalid characters
            self.job.addCaveats('invalid characters in stdout')

        # Remove NULL output and fail if it exists
        null_chars = ['\0', '\x00']
        for null_char in null_chars:
            if null_char in output:
                output = output.replace(null_char, 'NULL')
                if not self.job.isFail():
                    self.job.setStatus(self.job.error, 'NULL characters in output')

        return output

    def getExitCode(self):
        """
        Gets the error code of the process.
        """
        return self.exit_code

    def sendSignal(self, signal):
        """
        Sends a signal to the process.

        Can be overridden.
        """
        raise Exception('sendSignal not supported for this Runner')
